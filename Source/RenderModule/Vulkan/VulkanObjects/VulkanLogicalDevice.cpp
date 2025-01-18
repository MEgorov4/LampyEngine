#include "VulkanLogicalDevice.h"

#include <stdexcept>
#include <vector>
#include <iostream>
#include <set>
#include <format>

#include "../../../LoggerModule/Logger.h"


VulkanLogicalDevice::VulkanLogicalDevice(VkInstance instance, VkSurfaceKHR surface) : m_surface(surface)
{
	selectPhysicalDevice(instance, surface);
	createLogicalDevice();
}

void VulkanLogicalDevice::selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
	LOG_INFO("VulkanLogicalDevice: Start select physical device");

	uint32_t physicalDeviceCount = 0;

	if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to enumerate physical devices");
	}

	LOG_INFO("VulkanLogicalDevice: " + std::format("Physical devices count = {}", physicalDeviceCount));

	if (physicalDeviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(physicalDeviceCount);

	if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, &devices[0]) != VK_SUCCESS)
	{
		throw std::runtime_error("filed to enumerate physical devices to vector");
	}
	
	for (const VkPhysicalDevice& device : devices)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
	
		vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

		LOG_INFO("VulkanLogicalDevice: " + std::format("Device {} finded", physicalDeviceProperties.deviceName));

		if (isDeviceSuitable(device, surface))
		{
			LOG_INFO("VulkanLogicalDevice: Device " + std::format("{} selected", physicalDeviceProperties.deviceName));
			m_vk_physicalDevice = device;

			m_queueFamilyIndices = findQueueFamilies(device, surface);
			LOG_INFO("VulkanLogicalDevice: Queues = " + std::format("(Graphics queue index:{}, Present queue index:{})", m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.presentFamily.value()));

			m_swapChainSupportDetails = querySwapChainSupport(device, surface);

			return;
		}
	}

	throw std::runtime_error("failed to find suitable GPU!");
}

VulkanLogicalDevice::~VulkanLogicalDevice()
{
	if (m_vk_logicalDevice)
	{
		vkDestroyDevice(m_vk_logicalDevice, nullptr);
		LOG_INFO("VulkanLogicalDevice: Destroy logical device");
	}
}

bool VulkanLogicalDevice::isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	return queueFamilyIndices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool VulkanLogicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(VulkanLogicalDevice::deviceExtensions.begin(), VulkanLogicalDevice::deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices VulkanLogicalDevice::findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

SwapChainSupportDetails VulkanLogicalDevice::querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

void VulkanLogicalDevice::createLogicalDevice()
{
	LOG_INFO("VulkanLogicalDevice: Start create logical device");

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(m_vk_physicalDevice, &createInfo, nullptr, &m_vk_logicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}
	
	LOG_INFO("VulkanLogicalDevice: Logical device created");


	vkGetDeviceQueue(m_vk_logicalDevice, m_queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
	LOG_INFO("VulkanLogicalDevice: Get device graphics queue: " + std::format("Index = {}", m_queueFamilyIndices.graphicsFamily.value()));

	vkGetDeviceQueue(m_vk_logicalDevice, m_queueFamilyIndices.presentFamily.value(), 0, &m_presentQueue);
	LOG_INFO("VulkanLogicalDevice: Get device present queue: " + std::format("Index = {}", m_queueFamilyIndices.presentFamily.value()));
}
