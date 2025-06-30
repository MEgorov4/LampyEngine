#pragma once

#include <memory>

#include "../../EngineContext/IModule.h"
#include "../../EngineContext/ModuleRegistry.h"

namespace Logger
{
	class Logger;
}

namespace ResourceModule
{
	class ResourceManager;
}

namespace RenderModule
{
	class IRenderer;
	
	class RenderModule : public IModule
	{
		std::shared_ptr<Logger::Logger> m_logger;
		std::shared_ptr<ResourceModule::ResourceManager> m_resourceManager;
		
		std::unique_ptr<IRenderer> m_renderer; 

	public:
		void startup(const ModuleRegistry& registry) override;
		void shutdown() override;
		
		IRenderer* getRenderer();
	};
}
