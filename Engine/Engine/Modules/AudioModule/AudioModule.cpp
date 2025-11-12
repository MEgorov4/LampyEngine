#include "AudioModule.h"
#include <Foundation/Log/Log.h>
#include <Foundation/Log/LogVerbosity.h>

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
}

void AudioModule::playSoundAsync()
{
    m_audioThread = std::thread(
        []()
        {
        });
}
} // namespace AudioModule
