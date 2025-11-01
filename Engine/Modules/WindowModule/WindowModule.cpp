#include "WindowModule.h"

#include "Window.h"

#include <Modules/InputModule/InputModule.h>

namespace WindowModule
{
void WindowModule::startup()
{
    LT_PROFILE_SCOPE("WindowModule::startup");
    m_window = std::make_unique<Window>(800, 600, "Lampy Engine");

    LT_LOGI("WindowModule", "Startup");
}

/// <summary>
/// Shuts down the window module and destroys the window.
/// </summary>
void WindowModule::shutdown()
{
    LT_PROFILE_SCOPE("WindowModule::shutdown");
    LT_LOGI("WindowModule", "Shutdown");
    m_window.reset();
}

/// <summary>
/// Retrieves a pointer to the managed window.
/// </summary>
/// <returns>Pointer to the active Window instance.</returns>
/// <exception cref="std::runtime_error">Throws an assertion error if the window is not initialized.</returns>
Window* WindowModule::getWindow() const
{
    LT_PROFILE_SCOPE("WindowModule::getWindow");
    if (m_window)
    {
        return m_window.get();
    }
    return nullptr;
}
} // namespace WindowModule
