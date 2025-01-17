#pragma once 
#include <vulkan/vulkan.h>

class VulkanRenderPass {
    VkRenderPass m_vk_renderPass;
    VkDevice m_vk_device;

public:
    VulkanRenderPass(VkDevice device, VkFormat imageFormat);
    ~VulkanRenderPass();

    VkRenderPass getRenderPass() const { return m_vk_renderPass; }
};