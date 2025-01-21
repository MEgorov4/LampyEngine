#pragma once
#include "vulkan/vulkan.h"
#include <optional>
#include <vector>

struct QueueFamilyIndices 
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;

	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanLogicalDevice
{
	VkPhysicalDevice m_vk_physicalDevice;
	VkDevice m_vk_logicalDevice;
	
	VkSurfaceKHR m_surface;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	
	QueueFamilyIndices m_queueFamilyIndices;
	SwapChainSupportDetails m_swapChainSupportDetails;

	inline static const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
public:
	VulkanLogicalDevice(VkInstance instance, VkSurfaceKHR surface);
	~VulkanLogicalDevice();

	VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
	VkQueue getPresentQueue() const { return m_presentQueue; }
	
	VkPhysicalDevice getPhysicalDevice() const { return m_vk_physicalDevice; }
	VkDevice getLogicalDevice() const { return m_vk_logicalDevice; }

	QueueFamilyIndices getDeviceFamilyIndices() { return findQueueFamilies(m_vk_physicalDevice, m_surface); }
	SwapChainSupportDetails getDeviceSwapChainSupportDetails() { return querySwapChainSupport(m_vk_physicalDevice,  m_surface); }
	
	void deviceWaitIdle() const { vkDeviceWaitIdle(m_vk_logicalDevice); }
private:
	void selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice, VkSurfaceKHR surface);

	void createLogicalDevice();
};