#pragma once

#include <EngineMinimal.h>
#include "RenderContext.h"
namespace RenderModule
{
	class IRenderer;
	
	class RenderModule : public IModule
	{
		std::unique_ptr<IRenderer> m_renderer; 
		std::unique_ptr<RenderContext> m_context;
	public:
		void startup() override;
		void shutdown() override;
		
		IRenderer* getRenderer();
	};
}
