#include "OpenGLRenderer.h"

#include <format>
#include <imgui.h>
#include <glm/gtx/string_cast.hpp>

#include "../../LoggerModule/Logger.h"
#include "../../WindowModule/Window.h"
#include "../../ImGuiModule/OpenGLBackends/imgui_impl_opengl3.h"
#include "../../ImGuiModule/SDLBackends/imgui_impl_sdl3.h"
#include "../../ResourceModule/ResourceManager.h"

#include "OpenGLObjects/OpenGLFramebuffer.h"
#include "OpenGLObjects/OpenGLShader.h"
#include "OpenGLObjects/OpenGLMesh.h"
#include "OpenGLObjects/OpenGLMesh2D.h"
#include "OpenGLObjects/OpenGLTexture.h"

namespace RenderModule::OpenGL
{
    OpenGLRenderer::OpenGLRenderer(std::shared_ptr<Logger::Logger> logger,
                                   std::shared_ptr<ResourceModule::ResourceManager> resourceManager,
                                   std::shared_ptr<ECSModule::ECSModule> ecsModule,
                                   WindowModule::Window* window) : IRenderer(logger, resourceManager, ecsModule),
                                                                   m_window(window)
    {
        init();
    }

    void OpenGLRenderer::init()
    {
        glewExperimental = GL_TRUE;
        
        GLenum err = glewInit();
        if (err != GLEW_OK) // Correct check for success
        {
            m_logger->log(Logger::LogVerbosity::Error,
                          std::format("Failed to initialize GLEW: {}", reinterpret_cast<const char*>(glewGetErrorString(err))),
                          "RenderModule_OpenGLRenderer");
            throw std::runtime_error("Failed to initialize GLEW");
        }

        if (!GLEW_ARB_gl_spirv || !GLEW_ARB_spirv_extensions)
        {
            throw std::runtime_error("Required SPIR-V OpenGL extensions are not supported!");
        }

        SDL_GL_SetSwapInterval(0);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        std::pair<int, int> size = m_window->getWindowSize();
        glViewport(0, 0, size.first, size.second);

        glClearColor(0, 0, 0, 1);

        m_logger->log(Logger::LogVerbosity::Info, "Create framebuffers", "RenderModule_OpenGLRenderer");
        m_offscreenFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);

        m_shadowFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080, true);
        m_reflectionFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);
        m_lightFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);
        m_textureFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);
        m_customFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);
        m_finalFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);

        m_quadMesh2D = std::make_unique<OpenGLMesh2D>();
        initImGui();
    }

    void OpenGLRenderer::initImGui()
    {
        m_logger->log(Logger::LogVerbosity::Info, "Init ImGUI", "RenderModule_OpenGLRenderer");
        ImGui::CreateContext();

        ImGui_ImplSDL3_InitForOpenGL(m_window->getSDLWindow(), m_window->getGLContext());

        ImGui_ImplOpenGL3_Init("#version 450");
    }

    void OpenGLRenderer::debugMessageHandle(const std::string& message) const
    {
        m_logger->log(Logger::LogVerbosity::Info, "OpenGL debug message: " + message, "RenderModule_OpenGLRenderer");
    }

    void OpenGLRenderer::render()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0, 0, 0, 1.0f);
        std::pair<int, int> size = m_window->getWindowSize();
        glViewport(0, 0, size.first, size.second);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        renderWorld();
        SDL_GL_SwapWindow(m_window->getSDLWindow());
    }

    void OpenGLRenderer::renderWorld()
    {
        glEnable(GL_DEPTH_TEST);

        //m_shadowFramebuffer->bind();
        //glClear(GL_DEPTH_BUFFER_BIT);
        //renderPass(m_activeRenderPipelineData.shadowPass);
        //m_shadowFramebuffer->unbind();

        //m_reflectionFramebuffer->bind();
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //renderPass(m_activeRenderPipelineData.reflectionPass);
        //m_reflectionFramebuffer->unbind();


        //m_lightFramebuffer->bind();
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //renderPass(m_activeRenderPipelineData.lightPass);
        //m_lightFramebuffer->unbind();

        m_textureFramebuffer->bind();
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderPass(m_activeRenderPipelineData.texturePass);
        m_textureFramebuffer->unbind();

        //m_customFramebuffer->bind();
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //renderPass(m_activeRenderPipelineData.customPass);
        //m_customFramebuffer->unbind();

        glDisable(GL_DEPTH_TEST);
        m_finalFramebuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        renderPass(m_activeRenderPipelineData.finalPass);
        m_finalFramebuffer->unbind();
        glEnable(GL_DEPTH_TEST);

        glDisable(GL_DEPTH_TEST);
    }

    void OpenGLRenderer::renderPass(const RenderPassData& renderPassData)
    {
        ShaderUniformBlock_CameraData cameraData =
        {
            m_activeRenderPipelineData.viewMatrix, m_activeRenderPipelineData.projMatrix,
            m_activeRenderPipelineData.cameraPosition
        };

        std::unordered_map<std::string, GLuint> textures;

        if (renderPassData.renderPassType == FINAL)
        {
            textures["objectAlbedo"] = m_textureFramebuffer->getColorTexture();
        }
        for (auto& [shader, meshes] : renderPassData.batches)
        {
            shader->use();

            if (renderPassData.renderPassType == FINAL)
            {
                shader->bindTextures(textures);
                m_quadMesh2D->draw();
            }
            else
            {
                if (shader->hasUniformBlock("CameraData"))
                {
                    shader->setUniformData("CameraData", &cameraData, sizeof(ShaderUniformBlock_CameraData));
                }
                if (shader->hasUniformBlock("DirectionalLightData"))
                {
                    ShaderUniformBlock_DirectionalLightData dirLightData =
                    {
                        m_activeRenderPipelineData.directionalLight.direction,
                        m_activeRenderPipelineData.directionalLight.color
                    };

                    shader->setUniformData("DirectionalLightData", &dirLightData,
                                           sizeof(ShaderUniformBlock_DirectionalLightData));
                }
                for (auto& [mesh, objects] : meshes)
                {
                    for (auto& object : objects)
                    {
                        if (shader->hasUniformBlock("ModelMatrices"))
                        {
                            ShaderUniformBlock_ModelData modelData{object.modelMatrix};
                            shader->setUniformData("ModelMatrices", &modelData, sizeof(ShaderUniformBlock_ModelData));
                            if (object.texture)
                            {
                                textures["albedoTexture"] = object.texture.get()->getTextureID();
                                shader->bindTextures(textures);
                            }
                            else
                            {
                                textures["albedoTexture"] = m_emptyTextureGeneric->getTextureID();
                                shader->bindTextures(textures);
                            }
                        }


                        if (mesh)
                        {
                            mesh->draw();
                        }
                        textures.clear();
                    }
                }
            }
        }
    }

    void* OpenGLRenderer::getOffscreenImageDescriptor()
    {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(m_textureFramebuffer->getColorTexture()));
    }

    void OpenGLRenderer::waitIdle()
    {
        glFinish();
    }

    void OpenGLRenderer::drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color)
    {
        static GLuint VAO = 0, VBO = 0;

        if (VAO == 0)
        {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);
        }

        GLfloat vertices[] = {
            from.x, from.y, from.z,
            to.x, to.y, to.z
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glBindVertexArray(VAO);

        m_debugLineShader->use();

        OpenGLShader* shaderOpenGL = dynamic_cast<OpenGLShader*>(m_debugLineShader.get());
        glUniform3fv(glGetUniformLocation(shaderOpenGL->getProgramID(), "lineColor"), 1, &color[0]);
        //LOG_INFO(std::format("{}", shaderOpenGL->getProgramID()));

        glDrawArrays(GL_LINES, 0, 2);

        glBindVertexArray(0);
    }
}
