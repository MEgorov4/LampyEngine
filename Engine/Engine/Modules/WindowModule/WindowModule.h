#pragma once

#include <EngineMinimal.h>

namespace InputModule
{
class InputModule;
}

namespace WindowModule
{
class Window;
/// <summary>
/// Singleton module responsible for managing the application's window lifecycle.
/// </summary>
class WindowModule : public IModule
{
    std::unique_ptr<Window> m_window; ///< Unique pointer to the managed window.

  public:
    /// <summary>
    /// Initializes the window module and creates a new window.
    /// </summary>
    /// <param name="width">The width of the window in pixels.</param>
    /// <param name="height">The height of the window in pixels.</param>
    /// <param name="title">The title of the window.</param>
    /// <exception cref="std::runtime_error">Thrown if window creation fails.</exception>
    ///
    void startup() override;

    /// <summary>
    /// Shuts down the window module and destroys the window.
    /// </summary>
    void shutdown() override;

    /// <summary>
    /// Retrieves a pointer to the managed window.
    /// </summary>
    /// <returns>Pointer to the active Window instance.</returns>
    /// <exception cref="std::runtime_error">Throws an assertion error if the window is not initialized.</returns>
    Window* getWindow() const;
};
} // namespace WindowModule
