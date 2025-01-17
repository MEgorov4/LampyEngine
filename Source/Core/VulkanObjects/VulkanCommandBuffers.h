#pragma once 
#include <vulkan/vulkan.h>
#include <vector>

class VulkanCommandBuffers
{
	std::vector<VkCommandBuffer> m_vk_commandBuffers;

public:
	VulkanCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t imageCount);

	const std::vector<VkCommandBuffer>& getCommandBuffers() const { return m_vk_commandBuffers; };
};