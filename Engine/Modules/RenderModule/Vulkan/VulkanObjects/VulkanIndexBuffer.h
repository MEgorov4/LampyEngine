#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanBuffer.h"


class VulkanIndexBuffer : public VulkanBuffer
{
    uint32_t m_indexCount;

public:
    VulkanIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
        const std::vector<uint32_t>& indexData,
        VkQueue transferQueue, VkCommandPool commandPool);

    /// <summary>
    /// Destroys the vertex buffer and releases allocated memory.
    /// </summary>
    ~VulkanIndexBuffer();

    /// <summary>
    /// Retrieves the number of vertices stored in the buffer.
    /// </summary>
    /// <returns>The number of vertices in the buffer.</returns>
    uint32_t getIndexCount() { return m_indexCount; }
    
private:
    /// <summary>
    /// Creates a vertex buffer and uploads vertex data using a staging buffer.
    /// </summary>
    void createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
        const std::vector<uint32_t>& indexData,
        VkQueue transferQueue, VkCommandPool commandPool);
};