#pragma once

#include <vulkan/vulkan.h>
#include <vector>


class VulkanBuffer 
{
protected:
    VkDevice m_device; ///< Handle to the Vulkan logical device.
    VkBuffer m_buffer; ///< Handle to the Vulkan vertex buffer.
    VkDeviceMemory m_memory; ///< Handle to the allocated memory for the buffer.

public:
    VulkanBuffer(VkDevice device);
    void cleanupVulkanBuffer();

    virtual ~VulkanBuffer();

    VkBuffer getBuffer();

protected:
    void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
        VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool commandPool,
        VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};