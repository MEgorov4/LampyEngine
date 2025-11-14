#include "AudioRegister.h"

#include <Foundation/Log/LoggerMacro.h>

namespace ScriptModule
{
void AudioRegister::registerTypes(sol::state&, sol::environment& env)
{
    if (!m_service)
    {
        LT_LOGW("AudioRegister", "Audio service not available for scripting");
        return;
    }

    env.set_function("PlaySoundAsync", [svc = m_service]() { svc->playSoundAsync(); });
}
} // namespace ScriptModule

