#include "EventRegister.h"

#include <Foundation/Event/Event.h>

#include <SDL3/SDL_events.h>

namespace ScriptModule
{
void EventRegister::registerTypes(sol::state& state, sol::environment& env)
{
    state.new_usertype<EngineCore::Foundation::Event<SDL_KeyboardEvent>>(
        "EventKeyboard", sol::constructors<EngineCore::Foundation::Event<SDL_KeyboardEvent>()>(),
        "subscribe",
        [](EngineCore::Foundation::Event<SDL_KeyboardEvent>& self, sol::function luaHandler)
        { return self.subscribe([luaHandler](SDL_KeyboardEvent keyboardEvent) { luaHandler(keyboardEvent); }); },
        "unsubscribe", &EngineCore::Foundation::Event<SDL_KeyboardEvent>::unsubscribe,
        "invoke", &EngineCore::Foundation::Event<SDL_KeyboardEvent>::operator());

    sol::table envTable = env;
    envTable["EventKeyboard"]      = state["EventKeyboard"];
    envTable["EventKeyboardClass"] = state["EventKeyboard"];
}
} // namespace ScriptModule

