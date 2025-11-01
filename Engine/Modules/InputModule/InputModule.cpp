#include "InputModule.h"

namespace InputModule
{

void InputModule::startup()
{
    LT_PROFILE_SCOPE("InputModule::startup");
    LT_LOG(LogVerbosity::Info, "InputModule", "Startup");
}

void InputModule::shutdown()
{
    LT_PROFILE_SCOPE("InputModule::shutdown");
    LT_LOG(LogVerbosity::Info, "InputModule", "Shutdown");
}

void InputModule::pushEvent(const SDL_Event& event)
{
    LT_PROFILE_SCOPE("InputModule::pushEvent");
    OnEvent(event);
}

void InputModule::pushKeyboardEvent(const SDL_KeyboardEvent& event)
{
    LT_PROFILE_SCOPE("InputModule::pushKeyboardEvent");
    OnKeyboardEvent(event);
}

void InputModule::pushMouseButtonEvent(const SDL_MouseButtonEvent& event)
{
    LT_PROFILE_SCOPE("InputModule::pushMouseButtonEvent");
    OnMouseButtonEvent(event);
}

void InputModule::pushMouseMotionEvent(const SDL_MouseMotionEvent& event)
{
    LT_PROFILE_SCOPE("InputModule::pushMouseMotionEvent");
    OnMouseMotionEvent(event);
}

void InputModule::pushMouseWheelEvent(const SDL_MouseWheelEvent& event)
{
    LT_PROFILE_SCOPE("InputModule::pushMouseWheelEvent");
    OnMouseWheelEvent(event);
}
} // namespace InputModule
