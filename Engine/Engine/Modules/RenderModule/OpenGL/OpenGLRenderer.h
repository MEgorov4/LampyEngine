#pragma once

#include <EngineMinimal.h>

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
        OpenGLRenderer();

        void waitIdle() override;

    private:
        void init();

        void debugMessageHandle(const std::string& message) const;
    };
}
