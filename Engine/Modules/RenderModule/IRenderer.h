#pragma once

#include <vector>
#include <memory>

#include "Vulkan/VulkanObjects/Vertex.h"
#include "Abstract/RenderObject.h"
#include "Abstract/ITexture.h"

/// <summary>
/// Interface for a renderer, defining core rendering operations and resource management.
/// </summary>
class IRenderer
{

private:
    int m_onECSChanged;
    std::shared_ptr<IShader> m_shadowsShader;
    std::shared_ptr<IShader> m_reflectionsShader;
    std::shared_ptr<IShader> m_lightsShader;
    std::shared_ptr<IShader> m_finalShader;
    
    
protected:
    RenderPipelineData m_activeRenderPipelineData;
    RenderPipelineData m_updateRenderPipelineData;

    std::shared_ptr<ITexture> m_albedoGeneric;
    std::shared_ptr<ITexture> m_emissionGeneric;

    std::shared_ptr<IShader> m_debugLineShader;
public:
    /// <summary>
    /// Constructs an IRenderer with no assigned scene.
    /// </summary>
    IRenderer();

    /// <summary>
    /// Virtual destructor to ensure proper cleanup of derived renderers.
    /// </summary>
    virtual ~IRenderer();

    /// <summary>
    /// Renders a single frame.
    /// </summary>
    virtual void render() = 0;

    /// <summary>
    /// Registers a shader pipeline using the provided vertex and fragment shader paths.
    /// </summary>
    /// <param name="vertPath">Path to the vertex shader file.</param>
    /// <param name="fragPath">Path to the fragment shader file.</param>
    virtual void registerShader(const std::string& vertPath, const std::string& fragPath) = 0;

    /// <summary>
    /// Removes a previously registered shader pipeline.
    /// </summary>
    /// <param name="vertPath">Path to the vertex shader file.</param>
    /// <param name="fragPath">Path to the fragment shader file.</param>
    virtual void removeShader(const std::string& vertPath, const std::string& fragPath) = 0;

    /// <summary>
    /// Registers vertex data, creating a vertex buffer if needed.
    /// </summary>
    /// <param name="vertexData">Vertex data to register.</param>
    virtual void registerVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile) = 0;

    /// <summary>
    /// Removes vertex data, releasing associated buffers if no longer needed.
    /// </summary>
    /// <param name="vertexData">Vertex data to remove.</param>
    virtual void removeVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile) = 0;
    virtual void registerIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile) = 0;
    virtual void removeIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile) = 0;

    virtual void* getOffscreenImageDescriptor() = 0;
    /// <summary>
    /// Waits for the renderer to complete all rendering operations before proceeding.
    /// </summary>
    virtual void waitIdle() = 0;

    // DEBUG FUNCTION
    virtual void drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) = 0;

    void updateRenderList();
    void postInit();
};
