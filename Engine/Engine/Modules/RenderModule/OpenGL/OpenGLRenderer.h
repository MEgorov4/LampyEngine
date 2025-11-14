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

namespace RenderModule
{
    class IShader;
    class IMesh;
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
        void presentToWindow(TextureHandle handle) override;

    private:
        void init();
        std::shared_ptr<IShader> m_presentShader;
        std::shared_ptr<IMesh> m_presentQuad;

        void debugMessageHandle(const std::string& message) const;
    };
}
