#pragma once

#include <EngineMinimal.h>

class ALCdevice;
class ALCcontext;

namespace AudioModule
{
	class AudioModule : public IModule {
		ALCdevice* m_device = nullptr;
		ALCcontext* m_context = nullptr;
		std::thread m_audioThread;

	public:
		void startup() override;
		void shutdown() override;

		void playSoundAsync();
	};
}

