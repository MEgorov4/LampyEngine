#pragma once

#include <EngineMinimal.h>

namespace RenderModule
{
	class IRenderer;
	
	class RenderModule : public IModule
	{
		std::unique_ptr<IRenderer> m_renderer; 
	public:
		void startup() override;
		void shutdown() override;
		
		IRenderer* getRenderer();
	};
}
