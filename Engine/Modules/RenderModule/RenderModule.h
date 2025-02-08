#pragma once
#include <memory>
#include "RenderConfig.h"
#include "IRenderer.h"
#include "Vulkan/VulkanRenderer.h"
#include "../LoggerModule/Logger.h"

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
    void startUp(Window* window)
    {
        const RenderConfig& config = RenderConfig::getInstance();
        LOG_INFO(std::format("RenderModule: Startup with - {}", config.getGraphicsAPI() == GraphicsAPI::Vulkan ? "Vulkan" : "OpenGL"));

        switch (config.getGraphicsAPI())
        {
        case GraphicsAPI::Vulkan:
            m_renderer = std::make_unique<VulkanRenderer>(window);
            break;

            // case GraphicsAPI::OpenGL:
            //     m_renderer = std::make_unique<OpenGLRenderer>(); (Implementation required)
        }
    }

    /// <summary>
    /// Registers a shader pipeline using the provided vertex and fragment shader paths.
    /// </summary>
    /// <param name="vertPath">Path to the vertex shader file.</param>
    /// <param name="fragPath">Path to the fragment shader file.</param>
    void registerShader(const std::string& vertPath, const std::string& fragPath)
    {
        m_renderer->registerShader(vertPath, fragPath);
    }

    /// <summary>
    /// Removes a previously registered shader pipeline.
    /// </summary>
    /// <param name="vertPath">Path to the vertex shader file.</param>
    /// <param name="fragPath">Path to the fragment shader file.</param>
    void removeShader(const std::string& vertPath, const std::string& fragPath)
    {
        m_renderer->removeShader(vertPath, fragPath);
    }

    /// <summary>
    /// Registers vertex data, creating a Vulkan vertex buffer if needed.
    /// </summary>
    /// <param name="vertexData">Vertex data to register.</param>
    void registerVertexData(const std::vector<Vertex>& vertexData)
    {
        m_renderer->registerVertexData(vertexData);
    }

    /// <summary>
    /// Removes vertex data and releases associated buffers if no longer needed.
    /// </summary>
    /// <param name="vertexData">Vertex data to remove.</param>
    void removeVertexData(const std::vector<Vertex>& vertexData)
    {
        m_renderer->removeVertexData(vertexData);
    }
    
    VkDescriptorSet getVulkanOffscreenImageView();
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
