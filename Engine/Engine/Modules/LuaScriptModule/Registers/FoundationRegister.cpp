#include "FoundationRegister.h"

#include "../LuaScriptModule.h"

#include "../../../Foundation/Log/LoggerMacro.h"
#include "../../../Foundation/Log/LogVerbosity.h"
#include "../../../Foundation/Event/Event.h"

#include "../../InputModule/InputModule.h"
#include "../../AudioModule/AudioModule.h"

#include <SDL3/SDL_events.h>

namespace ScriptModule
{
void FoundationRegister::registerTypes(LuaScriptModule& module, sol::state& state)
{
    state.set_function("LogInfo", [](const std::string& msg) { LT_LOG(LogVerbosity::Info, "Script", msg); });
    state.set_function("LogDebug", [](const std::string& msg) { LT_LOG(LogVerbosity::Debug, "Script", msg); });
    state.set_function("LogVerbose", [](const std::string& msg) { LT_LOG(LogVerbosity::Verbose, "Script", msg); });
    state.set_function("LogWarning", [](const std::string& msg) { LT_LOG(LogVerbosity::Warning, "Script", msg); });
    state.set_function("LogError", [](const std::string& msg) { LT_LOG(LogVerbosity::Error, "Script", msg); });
    state.set_function("LogFatal", [](const std::string& msg) { LT_LOG(LogVerbosity::Fatal, "Script", msg); });

    state.new_usertype<Event<SDL_KeyboardEvent>>(
        "EventKeyboard", sol::constructors<Event<SDL_KeyboardEvent>()>(),
        "subscribe", [](Event<SDL_KeyboardEvent>& self, sol::function luaHandler)
        { return self.subscribe([luaHandler](SDL_KeyboardEvent keyboardEvent) { luaHandler(keyboardEvent); }); },
        "unsubscribe", &Event<SDL_KeyboardEvent>::unsubscribe,
        "invoke", &Event<SDL_KeyboardEvent>::operator());

    if (auto* input = module.getInputModule())
    {
        state["OnKeyboardEvent"] = &input->OnKeyboardEvent;
    }
}
} // namespace ScriptModule

