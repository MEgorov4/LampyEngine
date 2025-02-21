#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class IVulkanBuffer
{
protected:
    virtual void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
        VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    virtual uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    virtual void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool commandPool,
        VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};

class VulkanBuffer : public IVulkanBuffer
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
};