#pragma once

#include <memory>

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
    public:
        OpenGLRenderer(std::shared_ptr<Logger::Logger> logger,
                       std::shared_ptr<ResourceModule::ResourceManager> resourceManager,
                       std::shared_ptr<ECSModule::ECSModule> ecsModule,
                       std::shared_ptr<WindowModule::WindowModule> windowModule);

        void waitIdle() override;

    private:
        void init();

        void debugMessageHandle(const std::string& message) const;
    };
}
