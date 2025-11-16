#pragma once

#include <EngineMinimal.h>

union SDL_Event;

namespace RenderModule
{
/// <summary>
/// Interface for a UI rendering backend.
/// Implementations (e.g. Nuklear, RmlUi) receive input events and render UI using the active graphics API.
/// </summary>
class IUIRenderBackend
{
  public:
    virtual ~IUIRenderBackend() = default;

    /// <summary>
    /// Called at the beginning of a UI frame from the render module.
    /// </summary>
    virtual void beginFrame() noexcept = 0;

    /// <summary>
    /// Called at the end of a UI frame before rendering.
    /// </summary>
    virtual void endFrame() noexcept = 0;

    /// <summary>
    /// Performs the actual UI rendering using the underlying graphics API.
    /// </summary>
    virtual void render() noexcept = 0;

    /// <summary>
    /// Forwards an SDL event to the UI backend for input handling.
    /// </summary>
    virtual void processSDLEvent(const SDL_Event& evt) noexcept = 0;
};
} // namespace RenderModule


