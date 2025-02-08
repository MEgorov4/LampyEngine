#pragma once 
#include <vulkan/vulkan.h>

class VulkanDescriptorPool
{
	VkDescriptorPool m_vk_descriptorPool;
	VkDevice m_logicalDevice;
public:
	VulkanDescriptorPool(VkDevice m_logicalDevice);
	~VulkanDescriptorPool();

	VkDescriptorPool getDescriptorPool();
};