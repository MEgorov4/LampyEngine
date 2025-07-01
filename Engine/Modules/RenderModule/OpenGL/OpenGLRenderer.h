#pragma once

#include <memory>
#include <GL/glew.h>

#include "../IRenderer.h"

namespace WindowModule
{
    class Window;
}

namespace ResourceModule
{
    class ResourceManager;
}

namespace RenderModule::OpenGL
{
    class OpenGLFramebuffer;
    class OpenGLShader;
    class OpenGLVertexBuffer;
    class OpenGLMesh2D;

    class OpenGLRenderer : public IRenderer
    {
        WindowModule::Window* m_window;

        std::unique_ptr<OpenGLFramebuffer> m_offscreenFramebuffer;
        std::unique_ptr<OpenGLFramebuffer> m_shadowFramebuffer;
        std::unique_ptr<OpenGLFramebuffer> m_reflectionFramebuffer;
        std::unique_ptr<OpenGLFramebuffer> m_lightFramebuffer;
        std::unique_ptr<OpenGLFramebuffer> m_finalFramebuffer;
        std::unique_ptr<OpenGLFramebuffer> m_textureFramebuffer;
        std::unique_ptr<OpenGLFramebuffer> m_customFramebuffer;

        std::unique_ptr<OpenGLMesh2D> m_quadMesh2D;

        GLuint m_quadVAO;
        GLuint m_quadVBO;

    public:
        OpenGLRenderer(std::shared_ptr<Logger::Logger> logger,
            std::shared_ptr<ResourceModule::ResourceManager> resourceManager,
                       std::shared_ptr<ECSModule::ECSModule> ecsModule, WindowModule::Window* window);

        void render() override;

        void* getOffscreenImageDescriptor() override;

        void waitIdle() override;

        virtual void drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) override;

    private:
        void init();
        void initImGui();

        void debugMessageHandle(const std::string& message) const;

        void renderWorld();

        void renderPass(const RenderPassData& renderPassData);
    };
}
