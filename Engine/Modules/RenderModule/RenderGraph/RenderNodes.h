#pragma once

#include "../Abstract/IFramebuffer.h"
#include "../Abstract/IMesh.h"
#include "../Abstract/IShader.h"
#include "../Abstract/ITexture.h"
#include "../IRenderer.h"
#include "../OpenGL/OpenGLObjects/OpenGLMesh2D.h"
#include "../RenderContext.h"
#include "../RenderLocator.h" 

namespace RenderModule::RenderNodes
{
using Resource = RenderGraphResource;

// ---------------------------------------------------------
// 1️⃣ Shadow Pass
// ---------------------------------------------------------
inline void ShadowPass(const std::vector<Resource>& inputs, std::vector<Resource>& outputs)
{
    auto& ctx   = RenderLocator::Ref();
    auto& scene = ctx.scene();
    auto  rm    = ctx.resources();

    static std::shared_ptr<IShader> shader;
    static std::shared_ptr<IFramebuffer> fb;

    if (!shader)
    {
        shader = ShaderFactory::createOrGetShader(
            {"Shaders/GLSL/Core/shadow.vert"},
            {"Shaders/GLSL/Core/shadow.frag"});
    }

    if (!fb)
    {
        fb = FramebufferFactory::createOrGetFramebuffer({
            .height   = ctx.getViewport().second,
            .width    = ctx.getViewport().first,
            .useDepth = true,
            .name     = "ShadowPassFB"
        });
    }

    fb->bind();
    shader->use();

    for (auto& obj : scene.objects)
    {
        if (shader->hasUniformBlock("ModelMatrices"))
            shader->setUniformData("ModelMatrices", &obj.modelMatrix, sizeof(ShaderUniformBlock_ModelData));

        if (obj.mesh)
            obj.mesh->draw();
    }

    fb->unbind();
    outputs[0].handle = fb->getDepthTexture();
}

// ---------------------------------------------------------
// 2️⃣ Light Pass
// ---------------------------------------------------------
inline void LightPass(const std::vector<Resource>& inputs, std::vector<Resource>& outputs)
{
    auto& ctx   = RenderLocator::Ref();
    auto& scene = ctx.scene();
    auto  rm    = ctx.resources();

    static std::shared_ptr<IShader> shader;
    static std::shared_ptr<IFramebuffer> fb;

    if (!shader)
    {
        shader = ShaderFactory::createOrGetShader(
            ResourceModule::AssetID{"Shaders/GLSL/Core/light.vert"},
            ResourceModule::AssetID{"Shaders/GLSL/Core/light.frag"});
    }

    if (!fb)
    {
        fb = FramebufferFactory::createOrGetFramebuffer({
            .height   = ctx.getViewport().second,
            .width    = ctx.getViewport().first,
            .useDepth = true,
            .name     = "LightPassFB"
        });
    }

    fb->bind();
    shader->use();

    for (auto& obj : scene.objects)
    {
        if (shader->hasUniformBlock("ModelMatrices"))
            shader->setUniformData("ModelMatrices", &obj.modelMatrix, sizeof(ShaderUniformBlock_ModelData));

        if (obj.mesh)
            obj.mesh->draw();
    }

    fb->unbind();
    outputs[0].handle = fb->getColorTexture();
}

// ---------------------------------------------------------
// 3️⃣ Texture Pass
// ---------------------------------------------------------
inline void TexturePass(const std::vector<Resource>& inputs, std::vector<Resource>& outputs)
{
    auto& ctx   = RenderLocator::Ref();
    auto& scene = ctx.scene();
    auto  rm    = ctx.resources();

    static std::shared_ptr<IShader> shader;
    static std::shared_ptr<IFramebuffer> fb;
    static std::shared_ptr<ITexture> fallback;

    if (!shader)
    {
        shader = ShaderFactory::createOrGetShader(
            {"Shaders/GLSL/Core/texture.vert"},
            {"Shaders/GLSL/Core/texture.frag"});
        shader->scanTextureBindings({{"albedoTexture", 0}});
    }

    if (!fb)
    {
        fb = FramebufferFactory::createOrGetFramebuffer({
            .height   = ctx.getViewport().second,
            .width    = ctx.getViewport().first,
            .useDepth = true,
            .name     = "TexturePassFB"
        });
    }

    if (!fallback)
    {
        fallback = TextureFactory::createOrGetTexture({"Textures/Generic/GrayBoxTexture.png"});
    }

    fb->bind();
    shader->use();

    for (auto& obj : scene.objects)
    {
        if (shader->hasUniformBlock("ModelMatrices"))
            shader->setUniformData("ModelMatrices", &obj.modelMatrix, sizeof(ShaderUniformBlock_ModelData));

        TextureHandle texHandle = fallback->getTextureID();
        if (obj.texture)
            texHandle = obj.texture->getTextureID();

        shader->bindTextures({{"albedoTexture", texHandle}});

        if (obj.mesh)
            obj.mesh->draw();
    }

    fb->unbind();
    outputs[0].handle = fb->getColorTexture();
}

// ---------------------------------------------------------
// 4️⃣ Final Pass
// ---------------------------------------------------------
inline void FinalCompose(const std::vector<Resource>& inputs, std::vector<Resource>& outputs)
{
    auto& ctx = RenderLocator::Ref();
    auto  rm  = ctx.resources();

    static std::shared_ptr<IShader> shader;
    static std::shared_ptr<IFramebuffer> fb;
    static std::shared_ptr<IMesh> quad;

    if (!shader)
    {
        shader = ShaderFactory::createOrGetShader(
            {"Shaders/GLSL/Core/final.vert"},
            {"Shaders/GLSL/Core/final.frag"});
        shader->scanTextureBindings({
            {"shadow_pass_depth", 0},
            {"light_pass_color", 1},
            {"texture_pass_color", 2}
        });
    }

    if (!fb)
    {
        fb = FramebufferFactory::createOrGetFramebuffer({
            .height   = ctx.getViewport().second,
            .width    = ctx.getViewport().first,
            .useDepth = true,
            .name     = "FinalPassFB"
        });
    }

    if (!quad)
    {
        quad = std::make_shared<OpenGL::OpenGLMesh2D>();
    }

    fb->bind();
    shader->use();

    std::unordered_map<std::string, TextureHandle> texBindings;
    for (auto& in : inputs)
        texBindings[in.name] = in.handle;

    shader->bindTextures(texBindings);
    quad->draw();

    fb->unbind();
    outputs[0].handle = fb->getColorTexture();
}
} // namespace RenderModule::RenderNodes
