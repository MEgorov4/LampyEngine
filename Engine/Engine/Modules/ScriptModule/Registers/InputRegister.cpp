#include "InputRegister.h"

#include <Foundation/Log/LoggerMacro.h>

namespace ScriptModule
{
void InputRegister::registerTypes(sol::state&, sol::environment& env)
{
    if (!m_service)
    {
        LT_LOGW("InputRegister", "Input service not available for scripting");
        return;
    }

    sol::table envTable = env;
    envTable["OnKeyboardEvent"] = &m_service->keyboardEvent();
}
} // namespace ScriptModule

