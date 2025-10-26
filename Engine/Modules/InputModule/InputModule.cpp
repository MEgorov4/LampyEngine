#include "InputModule.h"

namespace InputModule
{

	void InputModule::startup()
	{
		LT_LOG(LogVerbosity::Info, "InputModule", "Startup");
	}

	void InputModule::shutdown()
	{
		LT_LOG(LogVerbosity::Info, "InputModule", "Shutdown");
	}

	void InputModule::pushEvent(const SDL_Event& event)
	{
		OnEvent(event);
	}

	void InputModule::pushKeyboardEvent(const SDL_KeyboardEvent& event)
	{
		OnKeyboardEvent(event);
	}

	void InputModule::pushMouseButtonEvent(const SDL_MouseButtonEvent& event)
	{
		OnMouseButtonEvent(event);
	}

	void InputModule::pushMouseMotionEvent(const SDL_MouseMotionEvent& event)
	{
		OnMouseMotionEvent(event);	
	}

	void InputModule::pushMouseWheelEvent(const SDL_MouseWheelEvent& event)
	{
		OnMouseWheelEvent(event);
	}
}
