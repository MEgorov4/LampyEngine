#pragma once

#include <memory>
#include <flecs/addons/cpp/world.hpp>

#include "../../ResourceModule/ResourceManager.h"
#include "../../ImGuiModule/ImGuiModule.h"
#include "IFramebuffer.h"
#include "IMaterial.h"
#include "IShader.h"
#include "ITexture.h"
#include "IMesh.h"

#include "RenderObject.h"
#include "RenderObjectParser.h"
#include "../OpenGL/OpenGLObjects/OpenGLMesh2D.h"

namespace RenderModule
{
    class IRenderPassStrategy
    {
    protected:
        std::shared_ptr<ResourceModule::ResourceManager> m_resourceManager;
        std::shared_ptr<IFramebuffer> m_framebuffer;
        std::pair<std::vector<RenderObject>, std::vector<RenderObject>> m_renderObjects;

    public:
        IRenderPassStrategy(
            const std::shared_ptr<ResourceModule::ResourceManager>& resourceManager) : m_resourceManager(
            resourceManager)
        {
        }

        virtual ~IRenderPassStrategy() = default;
        virtual void parseWorld(flecs::world& world) = 0;
        virtual std::unordered_map<std::string, TextureHandle> execute(const ShaderUniformBlock_CameraData& cameraData,
                                                                       const std::unordered_map<
                                                                           std::string, TextureHandle>&
                                                                       inputTextures) = 0;
        virtual void cleanup() = 0;

        virtual RenderPassType type() const = 0;
    };

    class ShadowPassStrategy final : public IRenderPassStrategy
    {
        std::shared_ptr<IShader> m_shadowsShader;

    public:
        ShadowPassStrategy(const std::shared_ptr<ResourceModule::ResourceManager>& resourceManager)
            : IRenderPassStrategy(resourceManager)
        {
            std::shared_ptr<ResourceModule::RShader> SSHRV = m_resourceManager->load<ResourceModule::RShader>(
                "../Resources/Shaders/GLSL/Core/shadow.vert");
            std::shared_ptr<ResourceModule::RShader> SSHRF = m_resourceManager->load<ResourceModule::RShader>(
                "../Resources/Shaders/GLSL/Core/shadow.frag");
            m_shadowsShader = ShaderFactory::createOrGetShader(SSHRV, SSHRF);

            m_framebuffer = FramebufferFactory::createOrGetFramebuffer({
                .height = 1920, .width = 1080, .useDepth = true,
                .name = "ShadowPassFramebuffer"
            });
        }

        ~ShadowPassStrategy() override = default;

        void cleanup() override
        {
        }

        RenderPassType type() const override
        {
            return SHADOW;
        }

        std::unordered_map<std::string, TextureHandle> execute(const ShaderUniformBlock_CameraData& cameraData,
                                                               const std::unordered_map<std::string, TextureHandle>&
                                                               inputTextures) override
        {
            m_framebuffer->bind();
            m_shadowsShader->use();

            if (m_shadowsShader->hasUniformBlock("CameraData"))
            {
                m_shadowsShader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
            }

            for (auto& renderObject : m_renderObjects.first)
            {
                if (m_shadowsShader->hasUniformBlock("ModelMatrices"))
                {
                    m_shadowsShader->setUniformData("ModelMatrices", &renderObject.modelMatrix,
                                                    sizeof(ShaderUniformBlock_ModelData));
                }

                if (renderObject.mesh)
                {
                    renderObject.mesh->draw();
                }
            }
            m_framebuffer->unbind();

            return {
                {"shadow_pass_depth", m_framebuffer->getDepthTexture()},
                {"shadow_pass_color", m_framebuffer->getColorTexture()}
            };
        }

        void parseWorld(flecs::world& world) override
        {
            m_renderObjects.second.clear();
            m_renderObjects.second = RenderObjectParser::parse<MeshesOnly>(world);
            std::swap(m_renderObjects.first, m_renderObjects.second);
        }
    };

    class ReflectionPassStrategy final : public IRenderPassStrategy
    {
        std::shared_ptr<IShader> m_reflectionShader;

    public:
        ReflectionPassStrategy(const std::shared_ptr<ResourceModule::ResourceManager>& resourceManager)
            : IRenderPassStrategy(resourceManager)
        {
            std::shared_ptr<ResourceModule::RShader> RSHRV = m_resourceManager->load<ResourceModule::RShader>(
                "../Resources/Shaders/GLSL/Core/reflection.vert");
            std::shared_ptr<ResourceModule::RShader> RSHRF = m_resourceManager->load<ResourceModule::RShader>(
                "../Resources/Shaders/GLSL/Core/reflection.frag");
            m_reflectionShader = ShaderFactory::createOrGetShader(RSHRV, RSHRF);

            m_framebuffer = FramebufferFactory::createOrGetFramebuffer({
                .height = 1920, .width = 1080, .useDepth = true,
                .name = "ReflectionPassFramebuffer"
            });
        }

        ~ReflectionPassStrategy() override = default;

        void cleanup() override
        {
        }

        RenderPassType type() const override
        {
            return SHADOW;
        }

        std::unordered_map<std::string, TextureHandle> execute(const ShaderUniformBlock_CameraData& cameraData,
                                                               const std::unordered_map<std::string, TextureHandle>&
                                                               inputTextures) override
        {
            m_framebuffer->bind();
            m_reflectionShader->use();

            if (m_reflectionShader->hasUniformBlock("CameraData"))
            {
                m_reflectionShader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
            }

            for (auto& renderObject : m_renderObjects.first)
            {
                if (m_reflectionShader->hasUniformBlock("ModelMatrices"))
                {
                    m_reflectionShader->setUniformData("ModelMatrices", &renderObject.modelMatrix,
                                                       sizeof(ShaderUniformBlock_ModelData));
                }

                if (renderObject.mesh)
                {
                    renderObject.mesh->draw();
                }
            }
            m_framebuffer->unbind();

            return {
                {"reflection_pass_color", m_framebuffer->getColorTexture()},
                {"reflection_pass_depth", m_framebuffer->getDepthTexture()}
            };
        }

        void parseWorld(flecs::world& world) override
        {
            m_renderObjects.second.clear();
            m_renderObjects.second = RenderObjectParser::parse<MeshesOnly>(world);
            std::swap(m_renderObjects.first, m_renderObjects.second);
        }
    };

    class LightPassStrategy final : public IRenderPassStrategy
    {
        std::shared_ptr<IShader> m_lightShader;

    public:
        LightPassStrategy(const std::shared_ptr<ResourceModule::ResourceManager>& resourceManager)
            : IRenderPassStrategy(resourceManager)
        {
            std::shared_ptr<ResourceModule::RShader> LSHRV = m_resourceManager->load<ResourceModule::RShader>(
                "../Resources/Shaders/GLSL/Core/light.vert");
            std::shared_ptr<ResourceModule::RShader> LSHRF = m_resourceManager->load<ResourceModule::RShader>(
                "../Resources/Shaders/GLSL/Core/light.frag");
            m_lightShader = ShaderFactory::createOrGetShader(LSHRV, LSHRF);
            m_framebuffer = FramebufferFactory::createOrGetFramebuffer({
                .height = 1920, .width = 1080, .useDepth = true,
                .name = "LightPassFramebuffer"
            });
        }

        ~LightPassStrategy() override = default;


        void cleanup() override
        {
        }

        RenderPassType type() const override
        {
            return LIGHT;
        }

        std::unordered_map<std::string, TextureHandle> execute(const ShaderUniformBlock_CameraData& cameraData,
                                                               const std::unordered_map<std::string, TextureHandle>&
                                                               inputTextures) override
        {
            m_framebuffer->bind();
            m_lightShader->use();

            if (m_lightShader->hasUniformBlock("CameraData"))
            {
                m_lightShader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
            }

            for (auto& renderObject : m_renderObjects.first)
            {
                if (m_lightShader->hasUniformBlock("ModelMatrices"))
                {
                    m_lightShader->setUniformData("ModelMatrices", &renderObject.modelMatrix,
                                                  sizeof(ShaderUniformBlock_ModelData));
                }

                if (renderObject.mesh)
                {
                    renderObject.mesh->draw();
                }
            }
            m_framebuffer->unbind();

            return {
                {"light_pass_color", m_framebuffer->getColorTexture()},
                {"light_pass_depth", m_framebuffer->getDepthTexture()}
            };
        }

        void parseWorld(flecs::world& world) override
        {
            m_renderObjects.second.clear();
            m_renderObjects.second = RenderObjectParser::parse<MeshesOnly>(world);
            std::swap(m_renderObjects.first, m_renderObjects.second);
        }
    };

    class TexturePassStrategy : public IRenderPassStrategy
    {
        std::shared_ptr<IShader> m_textureShader;
        std::shared_ptr<ITexture> m_genericTexture;

    public:
        TexturePassStrategy(const std::shared_ptr<ResourceModule::ResourceManager>& resourceManager)
            : IRenderPassStrategy(resourceManager)
        {
            std::shared_ptr<ResourceModule::RShader> TSHRV = m_resourceManager->load<
                ResourceModule::RShader>("../Resources/Shaders/GLSL/Core/texture.vert");
            std::shared_ptr<ResourceModule::RShader> TSHRF = m_resourceManager->load<
                ResourceModule::RShader>("../Resources/Shaders/GLSL/Core/texture.frag");
            m_textureShader = ShaderFactory::createOrGetShader(TSHRV, TSHRF);

            m_textureShader->scanTextureBindings({
                {"albedoTexture", 0}
            });
            std::shared_ptr<ResourceModule::RTexture> DGBT = m_resourceManager->load<ResourceModule::RTexture>(
                "../Resources/Textures/Generic/GrayBoxTexture.png");
            m_genericTexture = TextureFactory::createOrGetTexture(DGBT);
            m_framebuffer = FramebufferFactory::createOrGetFramebuffer({
                .height = 1920, .width = 1080, .useDepth = true,
                .name = "TexturePassFramebuffer"
            });
        }

        ~TexturePassStrategy() override = default;

        void cleanup() override
        {
        }

        RenderPassType type() const override
        {
            return TEXTURE;
        }

        std::unordered_map<std::string, TextureHandle> execute(const ShaderUniformBlock_CameraData& cameraData,
                                                               const std::unordered_map<std::string, TextureHandle>&
                                                               inputTextures) override
        {
            std::unordered_map<std::string, TextureHandle> textures = inputTextures;
            m_framebuffer->bind();
            m_textureShader->use();

            if (m_textureShader->hasUniformBlock("CameraData"))
            {
                m_textureShader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
            }

            for (auto& renderObject : m_renderObjects.first)
            {
                if (m_textureShader->hasUniformBlock("ModelMatrices"))
                {
                    m_textureShader->setUniformData("ModelMatrices", &renderObject.modelMatrix,
                                                    sizeof(ShaderUniformBlock_ModelData));
                }
                if (renderObject.texture)
                {
                    textures["albedoTexture"] = renderObject.texture->getTextureID();
                    m_textureShader->bindTextures(textures);
                }
                else
                {
                    textures["albedoTexture"] = m_genericTexture->getTextureID();
                }
                if (renderObject.mesh)
                {
                    renderObject.mesh->draw();
                }
            }
            m_framebuffer->unbind();

            return {
                {"texture_pass_color", m_framebuffer->getColorTexture()},
                {"texture_pass_depth", m_framebuffer->getDepthTexture()}
            };
        }

        void parseWorld(flecs::world& world) override
        {
            m_renderObjects.second.clear();
            m_renderObjects.second = RenderObjectParser::parse<AllWithoutDebug>(world);
            std::swap(m_renderObjects.first, m_renderObjects.second);
        }
    };

    class CustomPassStrategy final : public IRenderPassStrategy
    {
    public:
        explicit CustomPassStrategy(const std::shared_ptr<ResourceModule::ResourceManager>& resourceManager)
            : IRenderPassStrategy(resourceManager)
        {
            m_framebuffer = FramebufferFactory::createOrGetFramebuffer({
                .height = 1920, .width = 1080, .useDepth = true,
                .name = "ImGuiPassFramebuffer"
            });
        }

        ~CustomPassStrategy() override = default;


        void cleanup() override
        {
        }

        RenderPassType type() const override
        {
            return CUSTOM;
        }

        std::unordered_map<std::string, TextureHandle> execute(const ShaderUniformBlock_CameraData& cameraData,
                                                               const std::unordered_map<std::string, TextureHandle>&
                                                               inputTextures) override
        {
            return {};
        }

        void parseWorld(flecs::world& world) override
        {
            m_renderObjects.second.clear();
            m_renderObjects.second = RenderObjectParser::parse<AllWithoutDebug>(world);
            std::swap(m_renderObjects.first, m_renderObjects.second);
        }
    };

    class FinalPassStrategy : public IRenderPassStrategy
    {
        std::shared_ptr<IShader> m_finalShader;
        std::shared_ptr<IMesh> m_quadMesh;

    public:
        FinalPassStrategy(const std::shared_ptr<ResourceModule::ResourceManager>& resourceManager)
            : IRenderPassStrategy(resourceManager)
        {
            std::shared_ptr<ResourceModule::RShader> FSHRV = m_resourceManager->load<ResourceModule::RShader>(
                "../Resources/Shaders/GLSL/Core/final.vert");
            std::shared_ptr<ResourceModule::RShader> FSHRF = m_resourceManager->load<ResourceModule::RShader>(
                "../Resources/Shaders/GLSL/Core/final.frag");
            m_finalShader = ShaderFactory::createOrGetShader(FSHRV, FSHRF);
            m_finalShader->scanTextureBindings({
                {"shadow_pass_depth", 0},
                {"reflection_pass_depth", 1},
                {"light_pass_color", 2},
                {"texture_pass_color", 3}
            });

            m_framebuffer = FramebufferFactory::createOrGetFramebuffer({
                .height = 1080, .width = 1920, .useDepth = true, .name = "FinalPassFramebuffer"
            });
            m_quadMesh = std::make_shared<OpenGL::OpenGLMesh2D>();
        }

        ~FinalPassStrategy() override = default;

        void cleanup() override
        {
        }

        RenderPassType type() const override
        {
            return FINAL;
        }

        std::unordered_map<std::string, TextureHandle> execute(const ShaderUniformBlock_CameraData& cameraData,
                                                               const std::unordered_map<std::string, TextureHandle>&
                                                               inputTextures) override
        {
            m_framebuffer->bind();
            m_finalShader->use();

            m_finalShader->bindTextures(inputTextures);
            m_quadMesh->draw();

            m_framebuffer->unbind();

            return {
                {"final_pass_color", m_framebuffer->getColorTexture()},
                {"final_pass_depth", m_framebuffer->getDepthTexture()}
            };
        }

        void parseWorld(flecs::world& world) override
        {
        }
    };
}
