#include "AudioModule.h"
#include <Foundation/Log/Log.h>
#include <Foundation/Log/LogVerbosity.h>
#include <chrono>

namespace AudioModule
{
void AudioModule::startup()
{
    ZoneScopedN("AudioModule::startup");
}

void AudioModule::shutdown()
{
    ZoneScopedN("AudioModule::shutdown");
    LT_LOG(LogVerbosity::Info, "AudioModule", "shutdown");
    stopAudioWorker();
}

void AudioModule::playSoundAsync()
{
    stopAudioWorker();
    m_stopAudioThread.store(false);
    m_audioThread = std::thread(
        [this]()
        {
            while (!m_stopAudioThread.load(std::memory_order_acquire))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                break;
            }
        });
}

void AudioModule::stopAudioWorker()
{
    m_stopAudioThread.store(true, std::memory_order_release);
    if (m_audioThread.joinable())
    {
        m_audioThread.join();
    }
}
} // namespace AudioModule
