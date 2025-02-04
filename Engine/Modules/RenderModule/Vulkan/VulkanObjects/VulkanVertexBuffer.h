#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanGraphicsPipeline.h"

/// <summary>
/// Manages a Vulkan vertex buffer for storing vertex data on the GPU.
/// </summary>
class VulkanVertexBuffer {
    VkDevice m_device; ///< Handle to the Vulkan logical device.
    VkBuffer m_buffer; ///< Handle to the Vulkan vertex buffer.
    VkDeviceMemory m_memory; ///< Handle to the allocated memory for the buffer.
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
    /// Retrieves the Vulkan buffer handle.
    /// </summary>
    /// <returns>Handle to the Vulkan buffer.</returns>
    VkBuffer getBuffer();

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

    /// <summary>
    /// Creates a Vulkan buffer with the specified size, usage, and memory properties.
    /// </summary>
    /// <param name="device">The Vulkan logical device.</param>
    /// <param name="physicalDevice">The Vulkan physical device.</param>
    /// <param name="size">The size of the buffer in bytes.</param>
    /// <param name="usage">The usage flags for the buffer.</param>
    /// <param name="properties">The memory property flags.</param>
    /// <param name="buffer">Reference to store the created buffer handle.</param>
    /// <param name="bufferMemory">Reference to store the allocated memory handle.</param>
    void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
        VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    /// <summary>
    /// Finds a suitable memory type based on the given type filter and memory properties.
    /// </summary>
    /// <param name="physicalDevice">The Vulkan physical device.</param>
    /// <param name="typeFilter">The type filter bitmask.</param>
    /// <param name="properties">The desired memory properties.</param>
    /// <returns>The index of the suitable memory type.</returns>
    /// <exception cref="std::runtime_error">Thrown if a suitable memory type is not found.</exception>
    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /// <summary>
    /// Copies data from a source buffer to a destination buffer using a temporary command buffer.
    /// </summary>
    /// <param name="device">The Vulkan logical device.</param>
    /// <param name="transferQueue">The queue used for buffer transfers.</param>
    /// <param name="commandPool">The command pool used for temporary command buffers.</param>
    /// <param name="srcBuffer">The source buffer.</param>
    /// <param name="dstBuffer">The destination buffer.</param>
    /// <param name="size">The size of the data to copy in bytes.</param>
    void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool commandPool,
        VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
