#pragma once

#include <memory>

#include "../../EngineContext/IModule.h"

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
		Logger::Logger* m_logger;

		std::unique_ptr<IRenderer> m_renderer; 
	public:
		void startup() override;
		void shutdown() override;
		
		IRenderer* getRenderer();
	};
}
