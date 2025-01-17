#include "VulkanCommandBuffers.h"
#include "../VulkanApplicationConfig.h"
#include <stdexcept>
#include <iostream>
#include "format"

#include "../Logger/Logger.h"

VulkanCommandBuffers::VulkanCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t imageCount)
{
    LOG_INFO("VulkanCommandBuffers: Start create command buffers");
    LOG_INFO("VulkanCommandBuffers: Buffers count " + std::format("{}", imageCount));
    m_vk_commandBuffers.resize(imageCount);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    allocInfo.commandBufferCount = static_cast<uint32_t>(m_vk_commandBuffers.size());

    if (vkAllocateCommandBuffers(device, &allocInfo, m_vk_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    
    for (const auto buffer : m_vk_commandBuffers)
    {
        LOG_INFO("VulkanCommandBuffers: Command buffer created " + std::to_string(reinterpret_cast<uintptr_t>(buffer)));
    }
}
