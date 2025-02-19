#pragma once
#include <array>
#include <glm/glm.hpp>

#include "../../../ResourceModule/Mesh.h"

#include "vulkan/vulkan.h"

/// <summary>
/// Represents a single vertex structure used in Vulkan rendering.
/// </summary>
struct Vertex {
    glm::vec3 pos;    ///< Vertex position (x, y, z).
    glm::vec2 uv;     ///< Texture coordinates (u, v).
    glm::vec3 normal; ///< Normal vector (x, y, z).

    /// <summary>
    /// Default constructor initializes the vertex with zero values.
    /// </summary>
    Vertex() : pos(0.f), uv(0.f), normal(0.f) {}

    /// <summary>
    /// Constructs a Vertex from a MeshVertex structure.
    /// </summary>
    /// <param name="meshVertex">The mesh vertex data to copy from.</param>
    Vertex(const MeshVertex& meshVertex);

    /// <summary>
    /// Returns the Vulkan binding description for a vertex buffer.
    /// </summary>
    /// <returns>VkVertexInputBindingDescription structure.</returns>
    static VkVertexInputBindingDescription getBindingDescription();

    /// <summary>
    /// Returns the Vulkan attribute descriptions for a vertex buffer.
    /// </summary>
    /// <returns>An array of VkVertexInputAttributeDescription.</returns>
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};
