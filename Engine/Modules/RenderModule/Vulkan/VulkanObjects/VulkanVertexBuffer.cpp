#include "VulkanVertexBuffer.h"
#include <stdexcept>
#include <cstring>
#include "../../../LoggerModule/Logger.h"

VulkanVertexBuffer::VulkanVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
    const std::vector<Vertex>& vertexData,
    VkQueue transferQueue, VkCommandPool commandPool) : VulkanBuffer(device), m_verticesCount(static_cast<uint32_t>(vertexData.size()))
{
    createVertexBuffer(device, physicalDevice, vertexData, transferQueue, commandPool);
}

VulkanVertexBuffer::~VulkanVertexBuffer() 
{
    
}

void VulkanVertexBuffer::createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
    const std::vector<Vertex>& vertexData,
    VkQueue transferQueue, VkCommandPool commandPool) {

    LOG_INFO("VulkanVertexBuffer: Start create vertex buffer");

    VkDeviceSize bufferSize = sizeof(vertexData[0]) * vertexData.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(device, physicalDevice, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    LOG_INFO("VulkanVertexBuffer: Staging buffer created");

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertexData.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(device, physicalDevice, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_buffer, m_memory);

    LOG_INFO("VulkanVertexBuffer: GPU buffer created");
    copyBuffer(device, transferQueue, commandPool, stagingBuffer, m_buffer, bufferSize);
    
    LOG_INFO("VulkanVertexBuffer: Move data from staging buffer to GPU buffer");

    vkDestroyBuffer(device, stagingBuffer, nullptr);

    LOG_INFO("VulkanVertexBuffer: Destroy staging buffer");
    
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    LOG_INFO("VulkanVertexBuffer: Free staging buffer memory");
}
