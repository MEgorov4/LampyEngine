#pragma once
#include <cstdint>

/// <summary>
/// Enum representing the available graphics APIs.
/// </summary>
enum class GraphicsAPI : uint8_t
{
    Vulkan, 
    OpenGL  
};

/// <summary>
/// Singleton class that manages rendering configuration settings.
/// </summary>
class RenderConfig
{
    uint32_t MAX_FRAMES_IN_FLIGHT = 2; ///< Maximum number of frames that can be in flight at once.
    GraphicsAPI GRAPHICS_API = GraphicsAPI::Vulkan; ///< Selected graphics API.
    bool IMGUI_ENABLED = true; ///< Flag indicating whether ImGui is enabled.

public:
    /// <summary>
    /// Retrieves the singleton instance of the RenderConfig class.
    /// </summary>
    /// <returns>Reference to the RenderConfig instance.</returns>
    static RenderConfig& getInstance()
    {
        static RenderConfig config;
        return config;
    }

    /// <summary>
    /// Gets the maximum number of frames that can be in flight simultaneously.
    /// </summary>
    /// <returns>Maximum frames in flight.</returns>
    uint32_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }

    /// <summary>
    /// Gets the currently selected graphics API.
    /// </summary>
    /// <returns>Enum value representing the graphics API.</returns>
    GraphicsAPI getGraphicsAPI() const { return GRAPHICS_API; }

    /// <summary>
    /// Checks whether ImGui rendering is enabled.
    /// </summary>
    /// <returns>True if ImGui is enabled, otherwise false.</returns>
    bool getImGuiEnabled() const { return IMGUI_ENABLED; }

    /// <summary>
    /// Sets the maximum number of frames in flight.
    /// </summary>
    /// <param name="framesInFlight">The new maximum frame count.</param>
    void setMaxFramesInFlight(const uint32_t framesInFlight) { MAX_FRAMES_IN_FLIGHT = framesInFlight; }

    /// <summary>
    /// Sets the graphics API to be used.
    /// </summary>
    /// <param name="graphicsAPI">The graphics API to set.</param>
    void setGraphicsAPI(GraphicsAPI graphicsAPI) { GRAPHICS_API = graphicsAPI; }

    /// <summary>
    /// Enables or disables ImGui rendering.
    /// </summary>
    /// <param name="state">True to enable ImGui, false to disable.</param>
    void setImGuiEnabled(bool state) { IMGUI_ENABLED = state; }

private:
    /// <summary>
    /// Private constructor to enforce singleton pattern.
    /// </summary>
    RenderConfig() = default;

    /// <summary>
    /// Private destructor to prevent external deletion.
    /// </summary>
    ~RenderConfig() = default;

    /// <summary>
    /// Deleted copy constructor to prevent duplication.
    /// </summary>
    RenderConfig(const RenderConfig&) = delete;

    /// <summary>
    /// Deleted assignment operator to prevent reassignment.
    /// </summary>
    RenderConfig& operator=(const RenderConfig&) = delete;
};
