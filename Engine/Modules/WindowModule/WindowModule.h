#pragma once

#include <memory>
#include <stdexcept>
#include <cassert>

#include "Window.h"
#include "../LoggerModule/Logger.h"

/// <summary>
/// Singleton module responsible for managing the application's window lifecycle.
/// </summary>
class WindowModule
{
    std::unique_ptr<Window> m_window; ///< Unique pointer to the managed window.

public:
    /// <summary>
    /// Retrieves the singleton instance of the WindowModule.
    /// </summary>
    /// <returns>Reference to the WindowModule instance.</returns>
    static WindowModule& getInstance()
    {
        static WindowModule WindowModule;
        return WindowModule;
    }

    /// <summary>
    /// Initializes the window module and creates a new window.
    /// </summary>
    /// <param name="width">The width of the window in pixels.</param>
    /// <param name="height">The height of the window in pixels.</param>
    /// <param name="title">The title of the window.</param>
    /// <exception cref="std::runtime_error">Thrown if window creation fails.</exception>
    void startup(int width, int height, const char* title)
    {
        LOG_INFO("WindowModule: Startup");
        m_window = std::make_unique<Window>(width, height, title);
    }

    /// <summary>
    /// Shuts down the window module and destroys the window.
    /// </summary>
    void shutDown()
    {
        LOG_INFO("WindowModule: Shut down");
        m_window.reset();
    }

    /// <summary>
    /// Retrieves a pointer to the managed window.
    /// </summary>
    /// <returns>Pointer to the active Window instance.</returns>
    /// <exception cref="std::runtime_error">Throws an assertion error if the window is not initialized.</returns>
    Window* getWindow()
    {
        Window* window = m_window.get();
        assert(window && "WindowModule: No window instance available!");
        return window;
    }
};
