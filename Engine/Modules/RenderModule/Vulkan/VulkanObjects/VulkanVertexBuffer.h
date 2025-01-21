#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanGraphicsPipeline.h"

class VulkanVertexBuffer {
    VkDevice m_device;        
    VkBuffer m_buffer;        
    VkDeviceMemory m_memory;  

    uint32_t m_verticesCount;
public:
    VulkanVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
        const std::vector<Vertex>& vertexData,
        VkQueue transferQueue, VkCommandPool commandPool);
    ~VulkanVertexBuffer();

    VkBuffer getBuffer();

    uint32_t getVerticesCount() { return m_verticesCount; };

private:
    void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
        const std::vector<Vertex>& vertexData,
        VkQueue transferQueue, VkCommandPool commandPool);
    void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
        VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool commandPool,
        VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};

