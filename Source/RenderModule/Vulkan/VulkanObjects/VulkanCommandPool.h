#pragma once
#include <vulkan/vulkan.h>

class VulkanCommandPool {
    VkCommandPool m_vk_commandPool;
    VkDevice m_device;

public:
    VulkanCommandPool(VkDevice device, uint32_t queueFamilyIndex);
    ~VulkanCommandPool();

    VkCommandPool getCommandPool() const { return m_vk_commandPool; }
};