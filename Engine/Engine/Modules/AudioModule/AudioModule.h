#pragma once

#include <EngineMinimal.h>
#include <thread>
#include <atomic>

namespace AudioModule
{
class AudioModule : public IModule
{
    std::thread m_audioThread;
    std::atomic<bool> m_stopAudioThread{false};

  public:
    void startup() override;
    void shutdown() override;

    void playSoundAsync();

  private:
    void stopAudioWorker();
};
} // namespace AudioModule
