#pragma once

#include <vector>

#include "../ObjectCoreModule/ObjectModel/Scene.h"
#include "Vulkan/VulkanObjects/Vertex.h"
/// <summary>
/// Interface for a renderer, defining core rendering operations and resource management.
/// </summary>
class IRenderer
{
protected:
    Scene* m_rendererScene; ///< Pointer to the scene currently being rendered.

public:
    /// <summary>
    /// Constructs an IRenderer with no assigned scene.
    /// </summary>
    IRenderer() : m_rendererScene(nullptr) {}

    /// <summary>
    /// Virtual destructor to ensure proper cleanup of derived renderers.
    /// </summary>
    virtual ~IRenderer() {}

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
    virtual void registerVertexData(const std::vector<Vertex>& vertexData) = 0;

    /// <summary>
    /// Removes vertex data, releasing associated buffers if no longer needed.
    /// </summary>
    /// <param name="vertexData">Vertex data to remove.</param>
    virtual void removeVertexData(const std::vector<Vertex>& vertexData) = 0;

    /// <summary>
    /// Sets the scene that the renderer will render.
    /// </summary>
    /// <param name="scene">Pointer to the scene to render.</param>
    virtual void setSceneToRender(Scene* scene) { m_rendererScene = scene; }
    
    virtual void* getVulkanOffscreenImageView() = 0;
    /// <summary>
    /// Waits for the renderer to complete all rendering operations before proceeding.
    /// </summary>
    virtual void waitIdle() = 0;
};
