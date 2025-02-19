#include "VulkanIndexBuffer.h"
#include <stdexcept>
#include <cstring>
#include "../../../LoggerModule/Logger.h"

VulkanIndexBuffer::VulkanIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
    const std::vector<uint32_t>& indexData,
    VkQueue transferQueue, VkCommandPool commandPool) : m_indexCount(static_cast<uint32_t>(indexData.size()))
{
    createIndexBuffer(device, physicalDevice, indexData, transferQueue, commandPool);
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{

}

void VulkanIndexBuffer::createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
    const std::vector<uint32_t>& indexData,
    VkQueue transferQueue, VkCommandPool commandPool) {

    LOG_INFO("VulkanIndexBuffer: Start create Index buffer");

    VkDeviceSize bufferSize = sizeof(indexData[0]) * indexData.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(device, physicalDevice, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    LOG_INFO("VulkanIndexBuffer: Staging buffer created");

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indexData.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(device, physicalDevice, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_buffer, m_memory);

    LOG_INFO("VulkanIndexBuffer: GPU buffer created");
    copyBuffer(device, transferQueue, commandPool, stagingBuffer, m_buffer, bufferSize);

    LOG_INFO("VulkanIndexBuffer: Move data from staging buffer to GPU buffer");

    vkDestroyBuffer(device, stagingBuffer, nullptr);

    LOG_INFO("VulkanIndexBuffer: Destroy staging buffer");

    vkFreeMemory(device, stagingBufferMemory, nullptr);

    LOG_INFO("VulkanIndexBuffer: Free staging buffer memory");
}
