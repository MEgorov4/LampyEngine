#include "NetworkSendRegister.h"

#include <Foundation/Log/LoggerMacro.h>

namespace ScriptModule
{
void NetworkSendRegister::registerTypes(sol::state&, sol::environment& env)
{
    env.set_function("NetworkSend",
                     [](const std::string& channel, const std::string& payload)
                     {
                         LT_LOGI("ScriptNetworkSend",
                                 std::string("Send request on channel ") + channel + " payload: " + payload);
                     });
}
} // namespace ScriptModule

