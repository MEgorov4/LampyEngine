#include "InputModule.h"

namespace InputModule
{

void InputModule::startup()
{
    ZoneScopedN("InputModule::startup");
    LT_LOG(LogVerbosity::Info, "InputModule", "Startup");
}

void InputModule::shutdown()
{
    ZoneScopedN("InputModule::shutdown");
    LT_LOG(LogVerbosity::Info, "InputModule", "Shutdown");
}

void InputModule::pushEvent(const SDL_Event& event)
{
    ZoneScopedN("InputModule::pushEvent");
    OnEvent(event);
}

void InputModule::pushKeyboardEvent(const SDL_KeyboardEvent& event)
{
    ZoneScopedN("InputModule::pushKeyboardEvent");
    OnKeyboardEvent(event);
}

void InputModule::pushMouseButtonEvent(const SDL_MouseButtonEvent& event)
{
    ZoneScopedN("InputModule::pushMouseButtonEvent");
    OnMouseButtonEvent(event);
}

void InputModule::pushMouseMotionEvent(const SDL_MouseMotionEvent& event)
{
    ZoneScopedN("InputModule::pushMouseMotionEvent");
    OnMouseMotionEvent(event);
}

void InputModule::pushMouseWheelEvent(const SDL_MouseWheelEvent& event)
{
    ZoneScopedN("InputModule::pushMouseWheelEvent");
    OnMouseWheelEvent(event);
}
} // namespace InputModule
