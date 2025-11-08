#pragma once

#include <EngineMinimal.h>


namespace AudioModule
{
class AudioModule : public IModule
{
    std::thread m_audioThread;

  public:
    void startup() override;
    void shutdown() override;

    void playSoundAsync();
};
} // namespace AudioModule
