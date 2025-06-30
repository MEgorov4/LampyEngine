#pragma once

#include <memory>
#include <thread>

#include "../../EngineContext/IModule.h"
#include "../../EngineContext/ModuleRegistry.h"

namespace Logger
{
	class Logger;
}

class ALCdevice;
class ALCcontext;

namespace AudioModule
{
	class AudioModule : public IModule {

		std::shared_ptr<Logger::Logger> m_logger;
		ALCdevice* m_device = nullptr;
		ALCcontext* m_context = nullptr;
		std::thread m_audioThread;
		
	public:
		void startup(const ModuleRegistry& moduleRegistry) override;
		void shutdown() override;

		void playSoundAsync();
	};
}

