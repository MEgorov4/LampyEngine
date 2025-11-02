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
// 2️⃣ Light Pass
// ---------------------------------------------------------
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

// ---------------------------------------------------------
// 3️⃣ Texture Pass
// ---------------------------------------------------------
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
            {"light_pass_color", 2},
            {"texture_pass_color", 3}
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
    static std::shared_ptr<RenderModule::OpenGL::OpenGLDebugMesh> debugMesh;

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
        debugMesh = std::make_shared<RenderModule::OpenGL::OpenGLDebugMesh>();
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
    
    // Копируем depth buffer из TexturePass framebuffer
    if (g_texturePassFB)
    {
        auto* sourceFB = static_cast<OpenGL::OpenGLFramebuffer*>(g_texturePassFB.get());
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
        debugMesh->setSphereData(sphere.center, sphere.radius, 16);
        
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
        gridShader = RenderFactory::get().createShader({"Shaders/GLSL/Core/grid.vert"},
                                                        {"Shaders/GLSL/Core/grid.frag"});
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
        // Создаем mesh для сетки один раз
        // Используем временный debug mesh для создания буферов
        const float gridSize = 100.0f;
        const float gridStep = 1.0f;
        const float y = 0.0f;

        // Создаем временный debug mesh
        auto tempMesh = std::make_shared<OpenGL::OpenGLDebugMesh>();
        
        // Создаем линии через setLineData для каждой линии сетки
        // Но это будет медленно. Вместо этого создадим mesh вручную через OpenGL
        // Используем статический подход - создаем mesh один раз через прямой доступ
        
        // Для оптимизации создаем mesh с минимальным набором линий
        // Вместо 200+ линий создадим mesh напрямую
        gridMesh = std::make_shared<OpenGL::OpenGLDebugMesh>();
        
        // Устанавливаем первую линию чтобы инициализировать буферы
        static_cast<OpenGL::OpenGLDebugMesh*>(gridMesh.get())->setLineData(
            glm::vec3(-gridSize, y, -gridSize),
            glm::vec3(gridSize, y, gridSize)
        );
        
        // Теперь обновим буферы с полным набором линий
        std::vector<float> vertices;
        
        // Линии вдоль оси Z (параллельные оси X)
        for (float x = -gridSize; x <= gridSize; x += gridStep)
        {
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(-gridSize);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(gridSize);
        }

        // Линии вдоль оси X (параллельные оси Z)
        for (float z = -gridSize; z <= gridSize; z += gridStep)
        {
            vertices.push_back(-gridSize);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(gridSize);
            vertices.push_back(y);
            vertices.push_back(z);
        }

        // Используем публичный метод для установки вершин
        static_cast<OpenGL::OpenGLDebugMesh*>(gridMesh.get())->setVerticesData(
            vertices.data(), vertices.size() / 3);
    }

    fb->bind();

    // Копируем входную текстуру, если есть
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

        // Копируем color
        copyShader->use();
        copyShader->bindTextures({{"sourceTexture", inputs[0].handle}});
        RenderState::enableDepthTest(false);
        RenderState::setDepthMask(false);
        quad->draw();
        RenderState::setDepthMask(true);
    }
    
    // Копируем depth buffer из TexturePass framebuffer
    if (g_texturePassFB)
    {
        auto* sourceFB = static_cast<OpenGL::OpenGLFramebuffer*>(g_texturePassFB.get());
        auto* destFB = static_cast<OpenGL::OpenGLFramebuffer*>(fb.get());
        
        if (sourceFB && destFB)
        {
            const auto [vpWidth, vpHeight] = ctx.getViewport();
            RenderState::blitDepthBuffer(sourceFB->getFBO(), destFB->getFBO(), vpWidth, vpHeight);
        }
    }

    gridShader->use();

    // Устанавливаем данные камеры
    if (gridShader->hasUniformBlock("CameraData"))
    {
        ShaderUniformBlock_CameraData cameraData;
        cameraData.view = scene.camera.view;
        cameraData.projection = scene.camera.projection;
        cameraData.position = scene.camera.position;
        gridShader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
    }

    // Устанавливаем данные сетки
    if (gridShader->hasUniformBlock("GridData"))
    {
        struct GridData
        {
            alignas(16) float gridSize;
            alignas(16) float gridStep;
            alignas(16) float majorStep;
            alignas(16) glm::vec3 majorLineColor;
            alignas(16) glm::vec3 minorLineColor;
            alignas(16) glm::vec3 axisColorX;
            alignas(16) glm::vec3 axisColorZ;
        };
        GridData gridData;
        gridData.gridSize = 100.0f;
        gridData.gridStep = 1.0f;
        gridData.majorStep = 10.0f;
        gridData.majorLineColor = glm::vec3(0.5f, 0.5f, 0.5f);
        gridData.minorLineColor = glm::vec3(0.3f, 0.3f, 0.3f);
        gridData.axisColorX = glm::vec3(1.0f, 0.2f, 0.2f);
        gridData.axisColorZ = glm::vec3(0.2f, 0.2f, 1.0f);
        gridShader->setUniformData("GridData", &gridData, sizeof(GridData));
    }

    // Рисуем сетку с правильным depth testing
    // Используем GL_LEQUAL чтобы сетка рисовалась там, где нет объектов
    // и объекты перекрывали сетку
    auto state = RenderState::saveState();
    RenderState::enableDepthTest(true);
    RenderState::setDepthFunc(DepthFunc::LessEqual);  // Рисуем если depth <= существующего
    RenderState::setDepthMask(false);   // Не записываем в depth buffer (read-only)
    RenderState::setLineWidth(1.0f);
    gridMesh->draw();
    RenderState::restoreState(state);

    fb->unbind();
    outputs[0].handle = fb->getColorTexture();
}
} // namespace RenderModule::RenderNodes
