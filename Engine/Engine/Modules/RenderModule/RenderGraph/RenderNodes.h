#pragma once

#include <EngineMinimal.h>
#include "../Abstract/IFramebuffer.h"
#include "../Abstract/IMesh.h"
#include "../Abstract/IShader.h"
#include "../Abstract/ITexture.h"
#include "../RenderFactory.h"
#include "../OpenGL/OpenGLObjects/OpenGLMesh2D.h"
#include "../OpenGL/OpenGLObjects/OpenGLDebugMesh.h"
#include "../OpenGL/OpenGLObjects/OpenGLFramebuffer.h"
#include "../Abstract/RenderState.h"
#include "../RenderContext.h"
#include "../RenderLocator.h"
#include "RenderGraphTypes.h"
#include "Foundation/Profiler/ProfileAllocator.h"

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
// OpenGL headers must be included before TracyOpenGL.hpp
// Only include OpenGL when available (GPU profiling)
#if defined(__has_include) && __has_include(<GL/glew.h>)
#include <GL/glew.h>
#include <tracy/TracyOpenGL.hpp>
#define RENDER_NODES_HAS_OPENGL 1
#else
#define RENDER_NODES_HAS_OPENGL 0
#endif
#else
#define RENDER_NODES_HAS_OPENGL 0
#endif

// Safe macro for GPU zones - only calls TracyGpuZone if GPU profiler is ready
#if RENDER_NODES_HAS_OPENGL && defined(TRACY_ENABLE)
#define LT_TRACY_GPU_ZONE(name) \
    do { \
        auto* ctx = RenderLocator::Get(); \
        if (ctx && ctx->isGpuProfilerReady()) { \
            TracyGpuZone(name); \
        } \
    } while(0)
#else
#define LT_TRACY_GPU_ZONE(name) do {} while(0)
#endif

namespace RenderModule::RenderNodes
{
using Resource = RenderGraphResource;
using namespace OpenGL;
// ---------------------------------------------------------
// 1️⃣ Shadow Pass
// ---------------------------------------------------------
inline void ShadowPass(const std::vector<Resource, ProfileAllocator<Resource>> &inputs,
                       std::vector<Resource, ProfileAllocator<Resource>> &outputs)
{
    ZoneScopedN("RenderNodes::ShadowPass");
    LT_TRACY_GPU_ZONE("ShadowPass");
    auto &ctx = RenderLocator::Ref();
    auto &scene = ctx.scene();
    auto rm = ctx.resources();

    static std::shared_ptr<IShader> shader;
    static std::shared_ptr<IFramebuffer> fb;

    if (!shader)
    {

        shader =
            RenderFactory::get().createShader({"Shaders/GLSL/Core/shadow.vert"}, {"Shaders/GLSL/Core/shadow.frag"});
    }

    if (!fb)
    {
        fb = RenderFactory::get().createFramebuffer({.height = ctx.getViewport().second,
                                                     .width = ctx.getViewport().first,
                                                     .useDepth = true,
                                                     .name = "ShadowPassFB"});
    }

    // Обновляем размер framebuffer'а при изменении viewport
    const auto [vpWidth, vpHeight] = ctx.getViewport();
    fb->resize(vpWidth, vpHeight);

    fb->bind();
    shader->use();

    // Устанавливаем данные камеры света (для shadow pass используется light view/projection)
    if (shader->hasUniformBlock("CameraData"))
    {
        // shadow.vert ожидает lightView и lightProjection в uniform block CameraData
        ShaderUniformBlock_CameraData lightData;
        lightData.view = scene.sun.lightViewMatrix;
        lightData.projection = scene.sun.lightProjectionMatrix;
        lightData.position = scene.sun.direction; // Используем direction как position для совместимости
        shader->setUniformData("CameraData", &lightData, sizeof(ShaderUniformBlock_CameraData));
    }

    for (auto &obj : scene.objects)
    {
        // Устанавливаем model matrix через обычный uniform (per-object данные)
        shader->setUniformMatrix4("model", obj.modelMatrix.model);

        if (obj.mesh)
            obj.mesh->draw();
    }

    fb->unbind();
    outputs[0].handle = fb->getDepthTexture();
}

// ---------------------------------------------------------
// 2️⃣ Light Pass (DEPRECATED - больше не используется)
// ---------------------------------------------------------
// LightPass удалён - всё освещение теперь выполняется в PBRPass
// Оставлено закомментированным для справки
/*
inline void LightPass(const std::vector<Resource, ProfileAllocator<Resource>> &inputs,
                      std::vector<Resource, ProfileAllocator<Resource>> &outputs)
{
    ZoneScopedN("RenderNodes::LightPass");
    LT_TRACY_GPU_ZONE("LightPass");
    auto &ctx = RenderLocator::Ref();
    auto &scene = ctx.scene();
    auto rm = ctx.resources();

    static std::shared_ptr<IShader> shader;
    static std::shared_ptr<IFramebuffer> fb;

    if (!shader)
    {
        shader = RenderFactory::get().createShader(ResourceModule::AssetID{"Shaders/GLSL/Core/light.vert"},
                                                   ResourceModule::AssetID{"Shaders/GLSL/Core/light.frag"});
    }

    if (!fb)
    {
        fb = RenderFactory::get().createFramebuffer({.height = ctx.getViewport().second,
                                                     .width = ctx.getViewport().first,
                                                     .useDepth = true,
                                                     .name = "LightPassFB"});
    }

    // Обновляем размер framebuffer'а при изменении viewport
    const auto [vpWidth, vpHeight] = ctx.getViewport();
    fb->resize(vpWidth, vpHeight);

    fb->bind();
    shader->use();

    // Устанавливаем данные камеры
    if (shader->hasUniformBlock("CameraData"))
    {
        ShaderUniformBlock_CameraData cameraData;
        cameraData.view = scene.camera.view;
        cameraData.projection = scene.camera.projection;
        cameraData.position = scene.camera.position;
        shader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
    }

    // Устанавливаем данные направленного света
    if (shader->hasUniformBlock("DirectionalLightData"))
    {
        ShaderUniformBlock_DirectionalLightData lightData;
        lightData.lightDirection = scene.sun.direction;
        lightData.lightColor = scene.sun.color;
        lightData.lightIntensity = scene.sun.intensity;
        shader->setUniformData("DirectionalLightData", &lightData, sizeof(ShaderUniformBlock_DirectionalLightData));
    }

    // Устанавливаем PointLights через обычные uniform массивы (не UBO)
    int lightCount = std::min(static_cast<int>(scene.pointLights.size()), 100);
    if (lightCount > 0)
    {
        std::vector<glm::vec4> positions(lightCount);
        std::vector<glm::vec4> colors(lightCount);
        std::vector<float> intensities(lightCount);
        std::vector<float> innerRadii(lightCount);
        std::vector<float> outerRadii(lightCount);
        
        for (int i = 0; i < lightCount; ++i)
        {
            positions[i] = scene.pointLights[i].position;
            colors[i] = scene.pointLights[i].color;
            intensities[i] = scene.pointLights[i].intensity;
            innerRadii[i] = scene.pointLights[i].innerRadius;
            outerRadii[i] = scene.pointLights[i].outerRadius;
        }
        
        shader->setUniformInt("lightCount", lightCount);
        shader->setUniformVec4Array("pointPositions", positions.data(), lightCount);
        shader->setUniformVec4Array("pointColors", colors.data(), lightCount);
        shader->setUniformFloatArray("pointIntensities", intensities.data(), lightCount);
        shader->setUniformFloatArray("pointInnerRadii", innerRadii.data(), lightCount);
        shader->setUniformFloatArray("pointOuterRadii", outerRadii.data(), lightCount);
    }
    else
    {
        shader->setUniformInt("lightCount", 0);
    }

    for (auto &obj : scene.objects)
    {
        // Устанавливаем model matrix через обычный uniform (per-object данные)
        shader->setUniformMatrix4("model", obj.modelMatrix.model);

        if (obj.mesh)
            obj.mesh->draw();
    }

    fb->unbind();
    outputs[0].handle = fb->getColorTexture();
}
*/

// ---------------------------------------------------------
// 3️⃣ PBR Pass (объединяет TexturePass и LightPass)
// ---------------------------------------------------------
// Глобальная переменная для хранения framebuffer из PBRPass (для копирования depth)
static std::shared_ptr<IFramebuffer> g_pbrPassFB = nullptr;

inline void PBRPass(const std::vector<Resource, ProfileAllocator<Resource>> &inputs,
                    std::vector<Resource, ProfileAllocator<Resource>> &outputs)
{
    ZoneScopedN("RenderNodes::PBRPass");
    LT_TRACY_GPU_ZONE("PBRPass");
    auto &ctx = RenderLocator::Ref();
    auto &scene = ctx.scene();
    auto rm = ctx.resources();

    static std::shared_ptr<IShader> shader;
    static std::shared_ptr<ITexture> fallback;

    if (!shader)
    {
        shader = RenderFactory::get().createShader({"Shaders/GLSL/Core/texture.vert"}, {"Shaders/GLSL/Core/texture.frag"});
        shader->scanTextureBindings({{"albedoTexture", 0}, {"normalTexture", 1}, {"roughnessMetallicTexture", 2}, {"shadowMap", 3}});
    }

    if (!g_pbrPassFB)
    {
        g_pbrPassFB = RenderFactory::get().createFramebuffer({.height = ctx.getViewport().second,
                                                              .width = ctx.getViewport().first,
                                                              .useDepth = true,
                                                              .name = "PBRPassFB"});
    }

    // Обновляем размер framebuffer'а при изменении viewport
    const auto [vpWidth, vpHeight] = ctx.getViewport();
    g_pbrPassFB->resize(vpWidth, vpHeight);

    if (!fallback)
    {
        fallback = RenderFactory::get().createTexture({"Textures/Generic/GrayBoxTexture.png"});
    }

    g_pbrPassFB->bind();
    shader->use();

    // ---- Set debug mode (0 = normal rendering, 1-9 = debug visualization) ----
    // Can be controlled via render context or config in the future
    shader->setUniformInt("debugMode", 0);

    // ---- Camera + Light UBOs ----
    // Все данные передаются в world-space для корректных вычислений в PBR шейдере
    ShaderUniformBlock_CameraData cameraData;
    cameraData.view = scene.camera.view;
    cameraData.projection = scene.camera.projection;
    cameraData.position = scene.camera.position; // world-space position (без преобразований)
    
    // Optional: Validate camera data for NaN (helps catch world-space calculation errors)
    #ifdef LT_DEBUG
    {
        // Check camera position for NaN (most critical for world-space calculations)
        bool hasNaN = glm::any(glm::isnan(cameraData.position));
        if (hasNaN)
        {
            LT_LOGW("PBRPass", "CameraData.position contains NaN values - world-space calculations may be incorrect");
        }
    }
    #endif
    
    shader->setUniformData("CameraData", &cameraData, sizeof(cameraData));

    ShaderUniformBlock_DirectionalLightData dirLight;
    dirLight.lightDirection = scene.sun.direction; // world-space direction (нормализованный)
    dirLight.lightColor = scene.sun.color;
    dirLight.lightIntensity = scene.sun.intensity;
    
    // Optional: Validate light direction is normalized and not NaN
    #ifdef LT_DEBUG
    {
        bool hasNaN = glm::any(glm::isnan(dirLight.lightDirection)) ||
                      glm::any(glm::isnan(dirLight.lightColor));
        if (hasNaN)
        {
            LT_LOGW("PBRPass", "DirectionalLightData contains NaN values - world-space calculations may be incorrect");
        }
        // Check if direction is normalized (length should be ~1.0 for directional light)
        float dirLen = glm::length(glm::vec3(dirLight.lightDirection));
        if (glm::abs(dirLen - 1.0f) > 0.01f && dirLen > 0.001f) // Allow small tolerance
        {
            LT_LOGW("PBRPass", "Light direction is not normalized - this may cause incorrect lighting");
        }
    }
    #endif
    
    shader->setUniformData("DirectionalLightData", &dirLight, sizeof(dirLight));

    glm::mat4 lightSpaceMatrix = scene.sun.lightProjectionMatrix * scene.sun.lightViewMatrix;
    shader->setUniformData("LightSpaceMatrix", &lightSpaceMatrix, sizeof(glm::mat4));

    // ---- Point Lights ----
    // Позиции точечных источников в world-space
    int count = std::min(static_cast<int>(scene.pointLights.size()), 100);
    shader->setUniformInt("lightCount", count);
    if (count > 0)
    {
        std::vector<glm::vec4> pos(count), col(count);
        std::vector<float> intens(count), inner(count), outer(count);
        for (int i = 0; i < count; ++i)
        {
            pos[i] = scene.pointLights[i].position; // world-space position
            col[i] = scene.pointLights[i].color;
            intens[i] = scene.pointLights[i].intensity;
            inner[i] = scene.pointLights[i].innerRadius;
            outer[i] = scene.pointLights[i].outerRadius;
            
            // Optional: Validate point light data for NaN
            #ifdef LT_DEBUG
            {
                // Check point light position for NaN (most critical for world-space calculations)
                bool hasNaN = glm::any(glm::isnan(pos[i])) ||
                              glm::any(glm::isnan(col[i])) ||
                              (intens[i] != intens[i]) || // NaN check for float
                              (inner[i] != inner[i]) ||
                              (outer[i] != outer[i]);
                if (hasNaN)
                {
                    LT_LOGW("PBRPass", "Point light contains NaN values - world-space calculations may be incorrect");
                }
            }
            #endif
        }
        shader->setUniformVec4Array("pointPositions", pos.data(), count);
        shader->setUniformVec4Array("pointColors", col.data(), count);
        shader->setUniformFloatArray("pointIntensities", intens.data(), count);
        shader->setUniformFloatArray("pointInnerRadii", inner.data(), count);
        shader->setUniformFloatArray("pointOuterRadii", outer.data(), count);
    }

    // ---- Bind shadow map ----
    TextureHandle shadowMapHandle{0};
    for (const auto& input : inputs)
    {
        if (input.name == "shadow_pass_depth" && input.handle.id != 0)
        {
            shadowMapHandle = input.handle;
            break;
        }
    }

    // ---- Render objects ----
    for (auto &obj : scene.objects)
    {
        shader->setUniformMatrix4("model", obj.modelMatrix.model);
        
        // Compute normal matrix: transpose(inverse(model)) for world-space normal transformation
        // This correctly handles non-uniform scaling by preserving normal direction
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(obj.modelMatrix.model)));
        
        // Optional: Validate normalMatrix for NaN/Inf (helps catch calculation errors)
        #ifdef LT_DEBUG
        {
            bool hasNaN = false;
            bool hasInf = false;
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    float val = normalMatrix[i][j];
                    // NaN check: NaN is the only value that is not equal to itself
                    if (val != val) hasNaN = true;
                    // Inf check: check if value is extremely large (not a perfect check, but works for validation)
                    if (val != val || val > 1e10f || val < -1e10f) {
                        if (val == val) hasInf = true; // Only set hasInf if it's not NaN
                    }
                }
            }
            if (hasNaN || hasInf) {
                LT_LOGW("PBRPass", "normalMatrix contains NaN or Inf values - normal transformation may be incorrect");
            }
            
            // Optional: Test normal transformation with a unit normal
            // Transform a test normal (1,0,0) and check if it remains approximately unit length
            glm::vec3 testNormal(1.0f, 0.0f, 0.0f);
            glm::vec3 transformedNormal = normalMatrix * testNormal;
            float transformedLen = glm::length(transformedNormal);
            // Allow some tolerance for non-uniform scales (normal may not be exactly unit after transformation)
            // But it should not be zero or extremely large
            if (transformedLen < 0.001f || transformedLen > 1000.0f) {
                LT_LOGW("PBRPass", "normalMatrix produces extremely scaled normals (length: {}) - check model matrix scaling", transformedLen);
            }
        }
        #endif
        
        shader->setUniformMatrix3("normalMatrix", normalMatrix);

        // Настраиваем текстуры (проверяем наличие перед установкой MaterialData)
        std::unordered_map<std::string, TextureHandle> textures;

        // Альбедо текстура - из материала или fallback
        TextureHandle albedoHandle = fallback->getTextureID();
        if (obj.material && !obj.material->albedoTexture.empty())
        {
            auto albedoTex = RenderFactory::get().createTexture(obj.material->albedoTexture);
            if (albedoTex)
                albedoHandle = albedoTex->getTextureID();
        }
        else if (obj.texture)
        {
            albedoHandle = obj.texture->getTextureID();
        }
        textures["albedoTexture"] = albedoHandle;

        // Нормальная текстура - из материала
        // Если текстура не привязана, normalStrength должна быть 0, чтобы не использовать normal map
        bool hasNormalTexture = false;
        if (obj.material && !obj.material->normalTexture.empty())
        {
            auto normalTex = RenderFactory::get().createTexture(obj.material->normalTexture);
            if (normalTex && normalTex->getTextureID().id != 0)
            {
                textures["normalTexture"] = normalTex->getTextureID();
                hasNormalTexture = true;
            }
            else
            {
                textures["normalTexture"] = {0};
            }
        }
        else
        {
            textures["normalTexture"] = {0};
        }

        // Устанавливаем MaterialData uniform block
        // IMPORTANT: If normal texture is not bound, set normalStrength to 0 to disable normal map
        ShaderUniformBlock_MaterialData materialData;
        if (obj.material)
        {
            materialData.albedoColor = obj.material->albedoColor;
            materialData.roughness = obj.material->roughness;
            materialData.metallic = obj.material->metallic;
            // Only use normal map if texture is actually bound
            materialData.normalStrength = hasNormalTexture ? obj.material->normalStrength : 0.0f;
        }
        else
        {
            // Значения по умолчанию
            materialData.albedoColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            materialData.roughness = 0.5f;
            materialData.metallic = 0.0f;
            materialData.normalStrength = 0.0f; // No normal map by default
        }
        shader->setUniformData("MaterialData", &materialData, sizeof(ShaderUniformBlock_MaterialData));

        // Roughness/Metallic текстура - из материала
        if (obj.material && !obj.material->roughnessMetallicTexture.empty())
        {
            auto rmTex = RenderFactory::get().createTexture(obj.material->roughnessMetallicTexture);
            if (rmTex)
                textures["roughnessMetallicTexture"] = rmTex->getTextureID();
            else
                textures["roughnessMetallicTexture"] = {0};
        }
        else
        {
            textures["roughnessMetallicTexture"] = {0};
        }

        // Добавляем shadow map
        if (shadowMapHandle.id != 0)
        {
            textures["shadowMap"] = shadowMapHandle;
        }
        else
        {
            textures["shadowMap"] = {0};
        }

        shader->bindTextures(textures);

        if (obj.mesh)
            obj.mesh->draw();
    }

    g_pbrPassFB->unbind();
    outputs[0].handle = g_pbrPassFB->getColorTexture();
}

// ---------------------------------------------------------
// 3️⃣ Texture Pass (DEPRECATED - заменён на PBRPass)
// ---------------------------------------------------------
// TexturePass удалён - функциональность перенесена в PBRPass
// Оставлено закомментированным для справки
/*
// Глобальная переменная для хранения framebuffer из TexturePass (для копирования depth)
static std::shared_ptr<IFramebuffer> g_texturePassFB = nullptr;

inline void TexturePass(const std::vector<Resource, ProfileAllocator<Resource>> &inputs,
                        std::vector<Resource, ProfileAllocator<Resource>> &outputs)
{
    ZoneScopedN("RenderNodes::TexturePass");
    LT_TRACY_GPU_ZONE("TexturePass");
    auto &ctx = RenderLocator::Ref();
    auto &scene = ctx.scene();
    auto rm = ctx.resources();

    static std::shared_ptr<IShader> shader;
    static std::shared_ptr<ITexture> fallback;

    if (!shader)
    {
        shader =
            RenderFactory::get().createShader({"Shaders/GLSL/Core/texture.vert"}, {"Shaders/GLSL/Core/texture.frag"});
        shader->scanTextureBindings({{"albedoTexture", 0}, {"normalTexture", 1}, {"roughnessMetallicTexture", 2}, {"shadowMap", 3}});
    }

    if (!g_texturePassFB)
    {
        g_texturePassFB = RenderFactory::get().createFramebuffer({.height = ctx.getViewport().second,
                                                                   .width = ctx.getViewport().first,
                                                                   .useDepth = true,
                                                                   .name = "TexturePassFB"});
    }

    // Обновляем размер framebuffer'а при изменении viewport
    const auto [vpWidth, vpHeight] = ctx.getViewport();
    g_texturePassFB->resize(vpWidth, vpHeight);

    if (!fallback)
    {
        fallback = RenderFactory::get().createTexture({"Textures/Generic/GrayBoxTexture.png"});
    }

    g_texturePassFB->bind();
    shader->use();

    // Устанавливаем данные камеры ОДИН РАЗ для всех объектов (оптимизация)
    if (shader->hasUniformBlock("CameraData"))
    {
        ShaderUniformBlock_CameraData cameraData;
        cameraData.view = scene.camera.view;
        cameraData.projection = scene.camera.projection;
        cameraData.position = scene.camera.position;
        shader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
    }

    // Устанавливаем данные направленного света
    if (shader->hasUniformBlock("DirectionalLightData"))
    {
        ShaderUniformBlock_DirectionalLightData lightData;
        lightData.lightDirection = scene.sun.direction;
        lightData.lightColor = scene.sun.color;
        lightData.lightIntensity = scene.sun.intensity;
        shader->setUniformData("DirectionalLightData", &lightData, sizeof(ShaderUniformBlock_DirectionalLightData));
    }

    // Устанавливаем light space matrix для теней
    if (shader->hasUniformBlock("LightSpaceMatrix"))
    {
        glm::mat4 lightSpaceMatrix = scene.sun.lightProjectionMatrix * scene.sun.lightViewMatrix;
        shader->setUniformData("LightSpaceMatrix", &lightSpaceMatrix, sizeof(glm::mat4));
    }

    // Устанавливаем PointLights через обычные uniform массивы (не UBO)
    int lightCount = std::min(static_cast<int>(scene.pointLights.size()), 100);
    if (lightCount > 0)
    {
        std::vector<glm::vec4> positions(lightCount);
        std::vector<glm::vec4> colors(lightCount);
        std::vector<float> intensities(lightCount);
        std::vector<float> innerRadii(lightCount);
        std::vector<float> outerRadii(lightCount);
        
        for (int i = 0; i < lightCount; ++i)
        {
            positions[i] = scene.pointLights[i].position;
            colors[i] = scene.pointLights[i].color;
            intensities[i] = scene.pointLights[i].intensity;
            innerRadii[i] = scene.pointLights[i].innerRadius;
            outerRadii[i] = scene.pointLights[i].outerRadius;
        }
        
        shader->setUniformInt("lightCount", lightCount);
        shader->setUniformVec4Array("pointPositions", positions.data(), lightCount);
        shader->setUniformVec4Array("pointColors", colors.data(), lightCount);
        shader->setUniformFloatArray("pointIntensities", intensities.data(), lightCount);
        shader->setUniformFloatArray("pointInnerRadii", innerRadii.data(), lightCount);
        shader->setUniformFloatArray("pointOuterRadii", outerRadii.data(), lightCount);
    }
    else
    {
        shader->setUniformInt("lightCount", 0);
    }

    // Батчинг: группируем объекты по шейдеру для уменьшения переключений
    std::unordered_map<std::shared_ptr<IShader>, std::vector<const RenderObject*>> shaderBatches;
    for (const auto &obj : scene.objects)
    {
        std::shared_ptr<IShader> objShader = obj.shader ? obj.shader : shader;
        shaderBatches[objShader].push_back(&obj);
    }

    // Рендерим батчами
    for (auto &[batchShader, objects] : shaderBatches)
    {
        batchShader->use();
        
        // Устанавливаем CameraData для этого шейдера (если отличается от дефолтного)
        if (batchShader != shader && batchShader->hasUniformBlock("CameraData"))
        {
            ShaderUniformBlock_CameraData cameraData;
            cameraData.view = scene.camera.view;
            cameraData.projection = scene.camera.projection;
            cameraData.position = scene.camera.position;
            batchShader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
        }
        
        // Устанавливаем DirectionalLightData для этого шейдера
        if (batchShader->hasUniformBlock("DirectionalLightData"))
        {
            ShaderUniformBlock_DirectionalLightData lightData;
            lightData.lightDirection = scene.sun.direction;
            lightData.lightColor = scene.sun.color;
            lightData.lightIntensity = scene.sun.intensity;
            batchShader->setUniformData("DirectionalLightData", &lightData, sizeof(ShaderUniformBlock_DirectionalLightData));
        }
        
        // Устанавливаем LightSpaceMatrix для этого шейдера
        if (batchShader->hasUniformBlock("LightSpaceMatrix"))
        {
            glm::mat4 lightSpaceMatrix = scene.sun.lightProjectionMatrix * scene.sun.lightViewMatrix;
            batchShader->setUniformData("LightSpaceMatrix", &lightSpaceMatrix, sizeof(glm::mat4));
        }
        
        // Устанавливаем PointLights через обычные uniform массивы (не UBO)
        int lightCount = std::min(static_cast<int>(scene.pointLights.size()), 100);
        if (lightCount > 0)
        {
            std::vector<glm::vec4> positions(lightCount);
            std::vector<glm::vec4> colors(lightCount);
            std::vector<float> intensities(lightCount);
            std::vector<float> innerRadii(lightCount);
            std::vector<float> outerRadii(lightCount);
            
            for (int i = 0; i < lightCount; ++i)
            {
                positions[i] = scene.pointLights[i].position;
                colors[i] = scene.pointLights[i].color;
                intensities[i] = scene.pointLights[i].intensity;
                innerRadii[i] = scene.pointLights[i].innerRadius;
                outerRadii[i] = scene.pointLights[i].outerRadius;
            }
            
            batchShader->setUniformInt("lightCount", lightCount);
            batchShader->setUniformVec4Array("pointPositions", positions.data(), lightCount);
            batchShader->setUniformVec4Array("pointColors", colors.data(), lightCount);
            batchShader->setUniformFloatArray("pointIntensities", intensities.data(), lightCount);
            batchShader->setUniformFloatArray("pointInnerRadii", innerRadii.data(), lightCount);
            batchShader->setUniformFloatArray("pointOuterRadii", outerRadii.data(), lightCount);
        }
        else
        {
            batchShader->setUniformInt("lightCount", 0);
        }
        
        for (const auto *obj : objects)
        {
            // Устанавливаем model matrix через обычный uniform (per-object данные)
            batchShader->setUniformMatrix4("model", obj->modelMatrix.model);
            
            // Вычисляем и устанавливаем normalMatrix для texture.vert
            glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(obj->modelMatrix.model)));
            batchShader->setUniformMatrix3("normalMatrix", normalMatrix);

            // Устанавливаем MaterialData uniform block
            if (batchShader->hasUniformBlock("MaterialData"))
            {
                ShaderUniformBlock_MaterialData materialData;
                
                // Используем материал если есть, иначе значения по умолчанию
                if (obj->material)
                {
                    materialData.albedoColor = obj->material->albedoColor;
                    materialData.roughness = obj->material->roughness;
                    materialData.metallic = obj->material->metallic;
                    materialData.normalStrength = obj->material->normalStrength;
                }
                else
                {
                    // Значения по умолчанию
                    materialData.albedoColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                    materialData.roughness = 0.5f;
                    materialData.metallic = 0.0f;
                    materialData.normalStrength = 1.0f;
                }
                
                batchShader->setUniformData("MaterialData", &materialData, sizeof(ShaderUniformBlock_MaterialData));
            }

            // Настраиваем текстуры
            std::unordered_map<std::string, TextureHandle> textures;
            
            // Альбедо текстура - из материала или fallback
            TextureHandle albedoHandle = fallback->getTextureID();
            if (obj->material && !obj->material->albedoTexture.empty())
            {
                auto albedoTex = RenderFactory::get().createTexture(obj->material->albedoTexture);
                if (albedoTex)
                    albedoHandle = albedoTex->getTextureID();
            }
            else if (obj->texture)
            {
                albedoHandle = obj->texture->getTextureID();
            }
            textures["albedoTexture"] = albedoHandle;
            
            // Нормальная текстура - из материала
            if (obj->material && !obj->material->normalTexture.empty())
            {
                auto normalTex = RenderFactory::get().createTexture(obj->material->normalTexture);
                if (normalTex)
                    textures["normalTexture"] = normalTex->getTextureID();
                else
                    textures["normalTexture"] = {0};
            }
            else
            {
                textures["normalTexture"] = {0};
            }
            
            // Roughness/Metallic текстура - из материала
            if (obj->material && !obj->material->roughnessMetallicTexture.empty())
            {
                auto rmTex = RenderFactory::get().createTexture(obj->material->roughnessMetallicTexture);
                if (rmTex)
                    textures["roughnessMetallicTexture"] = rmTex->getTextureID();
                else
                    textures["roughnessMetallicTexture"] = {0};
            }
            else
            {
                textures["roughnessMetallicTexture"] = {0};
            }
            
            // Добавляем shadow map из inputs
            if (!inputs.empty())
            {
                // Ищем shadow_pass_depth в inputs
                for (const auto& input : inputs)
                {
                    if (input.name == "shadow_pass_depth" && input.handle.id != 0)
                    {
                        textures["shadowMap"] = input.handle;
                        break;
                    }
                }
            }
            
            batchShader->bindTextures(textures);

            if (obj->mesh)
                obj->mesh->draw();
        }
    }

    g_texturePassFB->unbind();
    outputs[0].handle = g_texturePassFB->getColorTexture();
}
*/

// ---------------------------------------------------------
// 4️⃣ Final Pass
// ---------------------------------------------------------
inline void FinalCompose(const std::vector<Resource, ProfileAllocator<Resource>> &inputs,
                         std::vector<Resource, ProfileAllocator<Resource>> &outputs)
{
    ZoneScopedN("RenderNodes::FinalCompose");
    LT_TRACY_GPU_ZONE("FinalCompose");
    auto &ctx = RenderLocator::Ref();
    auto rm = ctx.resources();

    static std::shared_ptr<IShader> shader;
    static std::shared_ptr<IFramebuffer> fb;
    static std::shared_ptr<IMesh> quad;

    if (!shader)
    {
        shader = RenderFactory::get().createShader({"Shaders/GLSL/Core/final.vert"}, {"Shaders/GLSL/Core/final.frag"});
        shader->scanTextureBindings({
            {"shadow_pass_depth", 0},
            {"optionalReflection", 1}, // reflection_pass_depth (опционально)
            {"texture_pass_color", 2}  // Основной PBR результат
        });
    }

    if (!fb)
    {
        fb = RenderFactory::get().createFramebuffer({.height = ctx.getViewport().second,
                                                     .width = ctx.getViewport().first,
                                                     .useDepth = true,
                                                     .name = "FinalPassFB"});
    }

    // Обновляем размер framebuffer'а при изменении viewport
    const auto [vpWidth, vpHeight] = ctx.getViewport();
    fb->resize(vpWidth, vpHeight);

    if (!quad)
    {
        quad = RenderFactory::get().createMesh2D();
    }

    fb->bind();
    shader->use();

    std::unordered_map<std::string, TextureHandle> texBindings;
    for (auto &in : inputs)
    {
        // Маппим debug_pass_color на texture_pass_color для совместимости с шейдером
        if (in.name == "debug_pass_color")
        {
            texBindings["texture_pass_color"] = in.handle;
        }
        else
        {
            texBindings[in.name] = in.handle;
        }
    }

    shader->bindTextures(texBindings);
    quad->draw();

    fb->unbind();
    outputs[0].handle = fb->getColorTexture();
}

// ---------------------------------------------------------
// 5️⃣ Debug Pass
// ---------------------------------------------------------
inline void DebugPass(const std::vector<Resource, ProfileAllocator<Resource>> &inputs,
                      std::vector<Resource, ProfileAllocator<Resource>> &outputs)
{
    ZoneScopedN("RenderNodes::DebugPass");
    LT_TRACY_GPU_ZONE("DebugPass");
    auto &ctx = RenderLocator::Ref();
    auto &scene = ctx.scene();
    auto rm = ctx.resources();

    // Всегда копируем входную текстуру в свой framebuffer, даже если нет отладочных примитивов
    // Это гарантирует, что мы работаем с правильной текстурой

    static std::shared_ptr<IShader> debugShader;
    static std::shared_ptr<IShader> copyShader;
    static std::shared_ptr<IFramebuffer> fb;
    static std::shared_ptr<IMesh> quad;

    static std::shared_ptr<OpenGLDebugMesh> debugMesh;

    if (!debugShader)
    {
        debugShader = RenderFactory::get().createShader({"Shaders/GLSL/Core/debugLine.vert"},
                                                        {"Shaders/GLSL/Core/debugLine.frag"});
    }

    if (!copyShader)
    {
        copyShader = RenderFactory::get().createShader({"Shaders/GLSL/Core/copyTexture.vert"},
                                                       {"Shaders/GLSL/Core/copyTexture.frag"});
        copyShader->scanTextureBindings({{"sourceTexture", 0}});
    }

    if (!fb)
    {
        fb = RenderFactory::get().createFramebuffer({.height = ctx.getViewport().second,
                                                     .width = ctx.getViewport().first,
                                                     .useDepth = true,
                                                     .name = "DebugPassFB"});
    }

    // Обновляем размер framebuffer'а при изменении viewport
    const auto [vpWidth, vpHeight] = ctx.getViewport();
    fb->resize(vpWidth, vpHeight);

    if (!quad)
    {
        quad = RenderFactory::get().createMesh2D();
    }

    if (!debugMesh)
    {
        debugMesh = std::make_shared<OpenGLDebugMesh>();
    }

    fb->bind();
    
    // Всегда копируем входную текстуру в framebuffer, рисуя quad
    if (!inputs.empty() && inputs[0].handle.id != 0)
    {
        copyShader->use();
        copyShader->bindTextures({{"sourceTexture", inputs[0].handle}});
        RenderState::enableDepthTest(false);
        RenderState::setDepthMask(false);
        quad->draw();
        RenderState::setDepthMask(true);
    }
    
    // Копируем depth buffer из PBRPass framebuffer
    if (g_pbrPassFB)
    {
        auto* sourceFB = static_cast<OpenGL::OpenGLFramebuffer*>(g_pbrPassFB.get());
        auto* destFB = static_cast<OpenGL::OpenGLFramebuffer*>(fb.get());
        
        if (sourceFB && destFB)
        {
            const auto [vpWidth, vpHeight] = ctx.getViewport();
            RenderState::blitDepthBuffer(sourceFB->getFBO(), destFB->getFBO(), vpWidth, vpHeight);
        }
    }
    
    // Если нет отладочных примитивов, просто выходим - текстура уже скопирована
    if (scene.debugLines.empty() && scene.debugBoxes.empty() && scene.debugSpheres.empty())
    {
        fb->unbind();
        outputs[0].handle = fb->getColorTexture();
        return;
    }
    
    debugShader->use();

    // Устанавливаем данные камеры
    if (debugShader->hasUniformBlock("CameraData"))
    {
        ShaderUniformBlock_CameraData cameraData;
        cameraData.view = scene.camera.view;
        cameraData.projection = scene.camera.projection;
        cameraData.position = scene.camera.position;
        debugShader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
    }

    // Настраиваем depth test для отладочных примитивов - они должны учитывать глубину
    auto state = RenderState::saveState();
    RenderState::enableDepthTest(true);
    RenderState::setDepthFunc(DepthFunc::LessEqual);
    RenderState::setDepthMask(false); // Read-only depth
    RenderState::enableCullFace(false);
    RenderState::setLineWidth(2.0f);

    // Рисуем линии
    for (const auto &line : scene.debugLines)
    {
        debugMesh->setLineData(line.from, line.to);
        
        // Устанавливаем цвет линии
        if (debugShader->hasUniformBlock("LineColorBlock"))
        {
            struct LineColorBlock
            {
                glm::vec3 lineColor;
            };
            LineColorBlock colorBlock;
            colorBlock.lineColor = line.color;
            debugShader->setUniformData("LineColorBlock", &colorBlock, sizeof(LineColorBlock));
        }

        // Устанавливаем model matrix (единичная для линий в мировых координатах)
        debugShader->setUniformMatrix4("model", glm::mat4(1.0f));

        debugMesh->draw();
    }

    // Рисуем боксы
    for (const auto &box : scene.debugBoxes)
    {
        debugMesh->setBoxData(box.center, box.size);
        
        if (debugShader->hasUniformBlock("LineColorBlock"))
        {
            struct LineColorBlock
            {
                glm::vec3 lineColor;
            };
            LineColorBlock colorBlock;
            colorBlock.lineColor = box.color;
            debugShader->setUniformData("LineColorBlock", &colorBlock, sizeof(LineColorBlock));
        }

        if (debugShader->hasUniformBlock("ModelMatrices"))
        {
            ShaderUniformBlock_ModelData modelData;
            modelData.model = glm::mat4(1.0f);
            debugShader->setUniformData("ModelMatrices", &modelData, sizeof(ShaderUniformBlock_ModelData));
        }

        debugMesh->draw();
    }

    // Рисуем сферы
    for (const auto &sphere : scene.debugSpheres)
    {
        debugMesh->setSphereData(sphere.center, sphere.radius, 6);
        
        if (debugShader->hasUniformBlock("LineColorBlock"))
        {
            struct LineColorBlock
            {
                glm::vec3 lineColor;
            };
            LineColorBlock colorBlock;
            colorBlock.lineColor = sphere.color;
            debugShader->setUniformData("LineColorBlock", &colorBlock, sizeof(LineColorBlock));
        }

        if (debugShader->hasUniformBlock("ModelMatrices"))
        {
            ShaderUniformBlock_ModelData modelData;
            modelData.model = glm::mat4(1.0f);
            debugShader->setUniformData("ModelMatrices", &modelData, sizeof(ShaderUniformBlock_ModelData));
        }

        debugMesh->draw();
    }

    // Восстанавливаем состояние рендеринга
    RenderState::restoreState(state);

    fb->unbind();
    outputs[0].handle = fb->getColorTexture();
}

// ---------------------------------------------------------
// 6️⃣ Grid Pass
// ---------------------------------------------------------
inline void GridPass(const std::vector<Resource, ProfileAllocator<Resource>> &inputs,
                     std::vector<Resource, ProfileAllocator<Resource>> &outputs)
{
    ZoneScopedN("RenderNodes::GridPass");
    LT_TRACY_GPU_ZONE("GridPass");
    auto &ctx = RenderLocator::Ref();
    auto &scene = ctx.scene();
    auto rm = ctx.resources();

    static std::shared_ptr<IShader> gridShader;
    static std::shared_ptr<IFramebuffer> fb;
    static std::shared_ptr<IMesh> gridMesh;

    if (!gridShader)
    {
        gridShader = RenderFactory::get().createShader({"Shaders/GLSL/Core/grid_infinite.vert"},
                                                        {"Shaders/GLSL/Core/grid_infinite.frag"});
    }

    if (!fb)
    {
        fb = RenderFactory::get().createFramebuffer({.height = ctx.getViewport().second,
                                                     .width = ctx.getViewport().first,
                                                     .useDepth = true,
                                                     .name = "GridPassFB"});
    }

    // Обновляем размер framebuffer'а при изменении viewport
    const auto [vpWidth, vpHeight] = ctx.getViewport();
    fb->resize(vpWidth, vpHeight);

    if (!gridMesh)
    {
        // Создаем простой quad 2×2 в плоскости XZ (Y=0) для бесконечной сетки
        gridMesh = std::make_shared<OpenGL::OpenGLDebugMesh>();
        static_cast<OpenGL::OpenGLDebugMesh*>(gridMesh.get())->setQuadData();
    }

    fb->bind();

    // ВАЖНО: Для теста рисуем сетку БЕЗ копирования входной текстуры
    // Если зеленый экран виден, значит quad рендерится, и проблема была в порядке операций
    gridShader->use();

    // Устанавливаем данные камеры (всё, что нужно для бесконечной сетки)
    if (gridShader->hasUniformBlock("CameraData"))
    {
        ShaderUniformBlock_CameraData cameraData;
        cameraData.view = scene.camera.view;
        cameraData.projection = scene.camera.projection;
        cameraData.position = scene.camera.position;
        gridShader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
    }

    // Рисуем сетку БЕЗ depth test для проверки видимости
    auto state = RenderState::saveState();
    RenderState::enableDepthTest(false); // Отключаем для теста
    RenderState::enableBlend(false);
    gridMesh->draw(); // Автоматически использует GL_TRIANGLE_STRIP для quad
    RenderState::restoreState(state);
    
    // Копируем входную текстуру (PBR результат) ПОВЕРХ сетки для теста
    if (!inputs.empty() && inputs[0].handle.id != 0)
    {
        static std::shared_ptr<IShader> copyShader;
        static std::shared_ptr<IMesh> quad;

        if (!copyShader)
        {
            copyShader = RenderFactory::get().createShader({"Shaders/GLSL/Core/copyTexture.vert"},
                                                           {"Shaders/GLSL/Core/copyTexture.frag"});
            copyShader->scanTextureBindings({{"sourceTexture", 0}});
        }

        if (!quad)
        {
            quad = RenderFactory::get().createMesh2D();
        }

        // Копируем color с blend для смешивания с сеткой
        copyShader->use();
        copyShader->bindTextures({{"sourceTexture", inputs[0].handle}});
        RenderState::enableDepthTest(false);
        RenderState::enableBlend(true);
        RenderState::setBlendFunc(BlendFunc::SrcAlpha, BlendFunc::OneMinusSrcAlpha);
        quad->draw();
    }

    fb->unbind();
    outputs[0].handle = fb->getColorTexture();
}
} // namespace RenderModule::RenderNodes
