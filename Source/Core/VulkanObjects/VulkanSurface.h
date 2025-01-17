#pragma once
#include "vulkan/vulkan.h"

class VulkanSurface
{
	VkInstance m_vk_instance;
	VkSurfaceKHR m_vk_surface;
public:
	VulkanSurface(VkInstance instance, VkSurfaceKHR surface);
	~VulkanSurface();

	VkSurfaceKHR getSurface() const { return m_vk_surface; }
};