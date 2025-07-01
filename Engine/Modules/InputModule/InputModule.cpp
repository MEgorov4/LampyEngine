#include "InputModule.h"

#include "../LoggerModule/Logger.h"
#include "../WindowModule/Window.h"
#include "../WindowModule/WindowModule.h"
namespace InputModule
{

	void InputModule::startup(const ModuleRegistry& registry)
	{
		m_logger = std::dynamic_pointer_cast<Logger::Logger>(registry.getModule("Logger"));
		m_logger->log(Logger::LogVerbosity::Info,"Startup", "InputModule");
	}

	void InputModule::shutdown()
	{
		m_logger->log(Logger::LogVerbosity::Info,"Shutdown", "InputModule");
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
