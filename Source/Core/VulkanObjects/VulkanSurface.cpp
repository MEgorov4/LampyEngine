#include "VulkanSurface.h"

VulkanSurface::VulkanSurface(VkInstance instance, VkSurfaceKHR surface) : m_vk_instance(instance), m_vk_surface(surface){}

VulkanSurface::~VulkanSurface()
{
	if (m_vk_surface)
	{
		vkDestroySurfaceKHR(m_vk_instance, m_vk_surface, nullptr);
	}
}
