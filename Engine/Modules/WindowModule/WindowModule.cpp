#include "WindowModule.h"


#include "../LoggerModule/Logger.h"
#include "../InputModule/InputModule.h"

#include "Window.h"

namespace WindowModule
{
	void WindowModule::startup(const ModuleRegistry& registry)
	{
		m_logger = std::dynamic_pointer_cast<Logger::Logger>(registry.getModule("Logger"));
		m_inputModule = std::dynamic_pointer_cast<InputModule::InputModule>(registry.getModule("InputModule"));
		m_window = std::make_unique<Window>(m_logger, m_inputModule, 800, 600, "Lampy Engine");
		
		m_logger->log(Logger::LogVerbosity::Info, "Startup", "WindowModule");
	}

	/// <summary>
	/// Shuts down the window module and destroys the window.
	/// </summary>
	void WindowModule::shutdown()
	{
		m_logger->log(Logger::LogVerbosity::Info, "Shutdown", "WindowModule");
		m_window.reset();
	}

	/// <summary>
	/// Retrieves a pointer to the managed window.
	/// </summary>
	/// <returns>Pointer to the active Window instance.</returns>
	/// <exception cref="std::runtime_error">Throws an assertion error if the window is not initialized.</returns>
	Window* WindowModule::getWindow() const
	{
		if (m_window)
		{
			return m_window.get();
		}
		return nullptr;
	}
}
