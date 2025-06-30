#include "InputModule.h"

#include "../LoggerModule/Logger.h"
#include "../WindowModule/Window.h"
#include "../WindowModule/WindowModule.h"
namespace InputModule
{

	void InputModule::startup(const ModuleRegistry& registry)
	{
		m_logger = std::dynamic_pointer_cast<Logger::Logger>(registry.getModule("Logger"));
		m_windowModule = std::dynamic_pointer_cast<WindowModule::WindowModule>(registry.getModule("WindowModule"));

		WindowModule::Window* window = m_windowModule->getWindow();
		m_logger->log(Logger::LogVerbosity::Info,"Set input callbacks", "InputModule");
		if (window)
		{
			window->setKeyCallback([](GLFWwindow* win, int key, int scancode, int action, int mods) {
				
				//OnKeyAction(key, scancode, action, mods);
				});

			window->setCursorPositionCallback([](GLFWwindow* win, double xpos, double ypos) {
				//OnMousePosAction(xpos, ypos);
				});

			window->setScrollCallback([](GLFWwindow* win, double xoffset, double yoffset) {
				//OnScrollAction(xoffset, yoffset);
				});
		}
		else
		{
			m_logger->log(Logger::LogVerbosity::Error, "Window instance is null", "InputModule");
		}
	}

	void InputModule::shutdown()
	{
	}
}
