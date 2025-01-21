#include "VulkanCommandPool.h"
#include <stdexcept>
#include "../../../LoggerModule/Logger.h"

VulkanCommandPool::VulkanCommandPool(VkDevice device, uint32_t queueFamilyIndex) : m_device(device)
{
    LOG_INFO("VulkanCommandPool: Start create command pool");
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &m_vk_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    } 
    LOG_INFO("VulkanCommandPool: Command pool created");
}

VulkanCommandPool::~VulkanCommandPool()
{
    vkDestroyCommandPool(m_device, m_vk_commandPool, nullptr);
    LOG_INFO("VulkanCommandPool: Command pool destroyed");
}
