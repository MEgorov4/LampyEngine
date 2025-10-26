#include "RenderModule.h"

#include "../LoggerModule/Logger.h"
#include "../WindowModule/WindowModule.h"
#include "../ResourceModule/ResourceManager.h"
#include "../ObjectCoreModule/ECS/ECSModule.h"

#include "OpenGL/OpenGLRenderer.h"
#include "../../EngineContext/CoreGlobal.h"

namespace RenderModule
{
    void RenderModule::startup()
    {
        m_logger = GCM(Logger::Logger);
        m_logger->log(Logger::LogVerbosity::Info, "Startup", "RenderModule");
        m_logger->log(Logger::LogVerbosity::Info, "Create renderer", "RenderModule");
        m_renderer = std::make_unique<OpenGL::OpenGLRenderer>();

        m_renderer->postInit();
    }

    IRenderer* RenderModule::getRenderer()
    {
        IRenderer* renderer = m_renderer.get();
        assert(renderer);
        return renderer;
    }

    /// <summary>
    /// Shuts down the rendering module and releases all resources.
    /// </summary>
    void RenderModule::shutdown()
    {
        m_logger->log(Logger::LogVerbosity::Info, "Shutdown", "RenderModule");
        m_logger->log(Logger::LogVerbosity::Info, "Destroy renderer", "RenderModule");
        m_renderer.reset();
    }
}
