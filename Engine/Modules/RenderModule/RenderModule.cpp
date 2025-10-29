#include "RenderModule.h"

#include <Modules/WindowModule/WindowModule.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>

#include "OpenGL/OpenGLRenderer.h"

namespace RenderModule
{
    void RenderModule::startup()
    {
        LT_LOGI("RenderModule", "Startup");
        LT_LOGI("RenderModule", "Create renderer");
        m_renderer = std::make_unique<OpenGL::OpenGLRenderer>();

        m_renderer->postInit();
    }

    IRenderer* RenderModule::getRenderer()
    {
        IRenderer* renderer = m_renderer.get();
        assert(renderer);
        return renderer;
    }

    void RenderModule::shutdown()
    {
        LT_LOGI("RenderModule", "Shutdown");
        LT_LOGI("RenderModule", "Destroy renderer");
        m_renderer.reset();
    }
}
