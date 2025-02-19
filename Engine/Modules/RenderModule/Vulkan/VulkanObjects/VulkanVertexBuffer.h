#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanBuffer.h"

#include "Vertex.h"
/// <summary>
/// Manages a Vulkan vertex buffer for storing vertex data on the GPU.
/// </summary>
class VulkanVertexBuffer : public VulkanBuffer
{
    uint32_t m_verticesCount; ///< Number of vertices stored in the buffer.

public:
    /// <summary>
    /// Constructs a vertex buffer and uploads vertex data to the GPU.
    /// </summary>
    /// <param name="device">The Vulkan logical device.</param>
    /// <param name="physicalDevice">The Vulkan physical device.</param>
    /// <param name="vertexData">The vertex data to upload.</param>
    /// <param name="transferQueue">The queue used for buffer transfers.</param>
    /// <param name="commandPool">The command pool used for temporary command buffers.</param>
    VulkanVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
        const std::vector<Vertex>& vertexData,
        VkQueue transferQueue, VkCommandPool commandPool);

    /// <summary>
    /// Destroys the vertex buffer and releases allocated memory.
    /// </summary>
    ~VulkanVertexBuffer();

    /// <summary>
    /// Retrieves the number of vertices stored in the buffer.
    /// </summary>
    /// <returns>The number of vertices in the buffer.</returns>
    uint32_t getVerticesCount() { return m_verticesCount; }

private:
    /// <summary>
    /// Creates a vertex buffer and uploads vertex data using a staging buffer.
    /// </summary>
    void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
        const std::vector<Vertex>& vertexData,
        VkQueue transferQueue, VkCommandPool commandPool);
};
