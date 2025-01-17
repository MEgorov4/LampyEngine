#include "VulkanVertexBuffer.h"
#include <stdexcept>
#include <cstring>

#include "../Logger/Logger.h"

VulkanVertexBuffer::VulkanVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
    const std::vector<Vertex>& vertexData,
    VkQueue transferQueue, VkCommandPool commandPool)
    : m_device(device), m_buffer(VK_NULL_HANDLE), m_memory(VK_NULL_HANDLE)
{
    createVertexBuffer(device, physicalDevice, vertexData, transferQueue, commandPool);
}

VulkanVertexBuffer::~VulkanVertexBuffer() {
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_memory, nullptr);
}

VkBuffer VulkanVertexBuffer::getBuffer() const {
    return m_buffer;
}

void VulkanVertexBuffer::createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
    const std::vector<Vertex>& vertexData,
    VkQueue transferQueue, VkCommandPool commandPool) {

    LOG_INFO("VulkanVertexBuffer: Start create vertex buffer");

    VkDeviceSize bufferSize = sizeof(vertexData[0]) * vertexData.size();

    // Создаем staging-буфер
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(device, physicalDevice, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    LOG_INFO("VulkanVertexBuffer: Staging buffer created");

    // Копируем данные в staging-буфер
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertexData.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stagingBufferMemory);

    // Создаем GPU-буфер
    createBuffer(device, physicalDevice, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_buffer, m_memory);

    LOG_INFO("VulkanVertexBuffer: GPU buffer created");
    // Копируем данные из staging-буфера в GPU-буфер
    copyBuffer(device, transferQueue, commandPool, stagingBuffer, m_buffer, bufferSize);
    
    LOG_INFO("VulkanVertexBuffer: Move data from staging buffer to GPU buffer");

    // Удаляем staging-буфер
    vkDestroyBuffer(device, stagingBuffer, nullptr);

    LOG_INFO("VulkanVertexBuffer: Destroy staging buffer");
    
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    LOG_INFO("VulkanVertexBuffer: Free staging buffer memory");

    m_verticesCount = vertexData.size();
}

void VulkanVertexBuffer::createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
    VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

uint32_t VulkanVertexBuffer::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanVertexBuffer::copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool commandPool,
    VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(transferQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

