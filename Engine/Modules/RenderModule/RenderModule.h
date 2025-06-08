#pragma once
#include <memory>
#include "RenderConfig.h"
#include "IRenderer.h"
#include "../LoggerModule/Logger.h"

class Window;
/// <summary>
/// Singleton class that manages the rendering module, handling the initialization, selection,
/// and shutdown of the appropriate rendering API (e.g., Vulkan, OpenGL).
/// </summary>
class RenderModule
{
    std::unique_ptr<IRenderer> m_renderer; ///< Pointer to the active renderer.

public:
    /// <summary>
    /// Retrieves the singleton instance of the RenderModule.
    /// </summary>
    /// <returns>Reference to the RenderModule instance.</returns>
    static RenderModule& getInstance()
    {
        static RenderModule renderModule;
        return renderModule;
    }

    /// <summary>
    /// Initializes the rendering module based on the selected graphics API.
    /// </summary>
    /// <param name="window">Pointer to the application window.</param>
    void startup(Window* window);
    /// <summary>
    /// Registers vertex data, creating a Vulkan vertex buffer if needed.
    /// </summary>

    /// <summary>
    /// Retrieves a pointer to the active renderer.
    /// </summary>
    /// <returns>Pointer to the active IRenderer instance.</returns>
    IRenderer* getRenderer()
    {
        IRenderer* renderer = m_renderer.get();
        assert(renderer && "Renderer is not initialized!");
        return renderer;
    }

    /// <summary>
    /// Shuts down the rendering module and releases all resources.
    /// </summary>
    void shutDown()
    {
        LOG_INFO("RenderModule: Shut down");
        m_renderer.reset();
    }
};
