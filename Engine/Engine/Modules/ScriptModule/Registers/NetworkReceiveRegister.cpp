#include "NetworkReceiveRegister.h"

#include <Foundation/Log/LoggerMacro.h>

namespace ScriptModule
{
void NetworkReceiveRegister::registerTypes(sol::state&, sol::environment& env)
{
    env.set_function("NetworkSubscribe",
                     [](const std::string& channel, sol::function handler)
                     {
                         LT_LOGI("ScriptNetworkReceive",
                                 std::string("Registered receive handler for channel ") + channel);
                         // Placeholder for actual subscription logic
                     });
}
} // namespace ScriptModule

