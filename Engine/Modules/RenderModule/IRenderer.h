#pragma once

#include <vector>

#include "../ObjectCoreModule/ObjectModel/Scene.h"
#include "Vulkan/VulkanObjects/Vertex.h"
#include "Abstract/RenderObject.h"
/// <summary>
/// Interface for a renderer, defining core rendering operations and resource management.
/// </summary>
class IRenderer
{
protected:
    std::vector<RenderObject> m_renderObjects;
public:
    /// <summary>
    /// Constructs an IRenderer with no assigned scene.
    /// </summary>
    IRenderer() = default;

    /// <summary>
    /// Virtual destructor to ensure proper cleanup of derived renderers.
    /// </summary>
    virtual ~IRenderer() = default;

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
};
