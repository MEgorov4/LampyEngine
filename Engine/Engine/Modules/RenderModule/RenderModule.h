#pragma once

#include <EngineMinimal.h>
#include "RenderContext.h"
#include "RenderConfig.h"
#include <optional>
namespace RenderModule
{
	class IRenderer;
	
	struct RenderModuleConfig
	{
		std::optional<RenderOutputMode> outputMode;
		std::optional<bool> debugPassEnabled;
		std::optional<bool> gridPassEnabled;
	};

	class RenderModule : public IModule
	{
		std::unique_ptr<IRenderer> m_renderer; 
		std::unique_ptr<RenderContext> m_context;
		std::optional<RenderOutputMode> m_configuredOutputMode;
		std::optional<bool> m_configuredDebugPassEnabled;
		std::optional<bool> m_configuredGridPassEnabled;
	public:
		void startup() override;
		void shutdown() override;
		
		IRenderer* getRenderer();

		void applyConfig(const RenderModuleConfig& config);
	};
}
