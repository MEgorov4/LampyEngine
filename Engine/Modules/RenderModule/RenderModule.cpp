#include "RenderModule.h"

#include "../LoggerModule/Logger.h"
#include "../WindowModule/WindowModule.h"
#include "../ResourceModule/ResourceManager.h"
#include "../ObjectCoreModule/ECS/ECSModule.h"

#include "OpenGL/OpenGLRenderer.h"

namespace RenderModule
{
	void RenderModule::startup(const ModuleRegistry& registry)
	{
		m_logger = std::dynamic_pointer_cast<Logger::Logger>(registry.getModule("Logger"));
		std::shared_ptr<WindowModule::WindowModule> windowModule = std::dynamic_pointer_cast<WindowModule::WindowModule>(registry.getModule("WindowModule"));
		std::shared_ptr<ResourceModule::ResourceManager> resourceManager = std::dynamic_pointer_cast<ResourceModule::ResourceManager>(registry.getModule("ResourceManager"));
		std::shared_ptr<ECSModule::ECSModule> ecsModule = std::dynamic_pointer_cast<ECSModule::ECSModule>(registry.getModule("ECSModule"));

		m_logger->log(Logger::LogVerbosity::Info, "Startup", "RenderModule");
		m_logger->log(Logger::LogVerbosity::Info, "Create renderer", "RenderModule");
		m_renderer = std::make_unique<OpenGL::OpenGLRenderer>(resourceManager, ecsModule,windowModule->getWindow());

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
