#include "WindowModule.h"

#include "Window.h"

#include <Modules/InputModule/InputModule.h>
#include <SDL3/SDL.h>

namespace WindowModule
{
void WindowModule::startup()
{
    ZoneScopedN("WindowModule::startup");
    LT_LOGI("WindowModule", "Startup");
    m_window = std::make_unique<Window>(800, 600, "Lampy Engine");
}

/// <summary>
/// Shuts down the window module and destroys the window.
/// </summary>
void WindowModule::shutdown()
{
    ZoneScopedN("WindowModule::shutdown");
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
    ZoneScopedN("WindowModule::getWindow");
    if (m_window)
    {
        return m_window.get();
    }
    return nullptr;
}
} // namespace WindowModule
