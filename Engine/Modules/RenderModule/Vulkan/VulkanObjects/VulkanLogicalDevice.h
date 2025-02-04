#pragma once
#include "vulkan/vulkan.h"
#include <optional>
#include <vector>
#include <set>

/// <summary>
/// Represents queue family indices required for Vulkan operations.
/// </summary>
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily; ///< Index of the graphics queue family.
    std::optional<uint32_t> presentFamily;  ///< Index of the presentation queue family.

    /// <summary>
    /// Checks if all required queue families are assigned.
    /// </summary>
    /// <returns>True if both graphics and present queue families are available.</returns>
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

/// <summary>
/// Stores details about swap chain support.
/// </summary>
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities; ///< Surface capabilities.
    std::vector<VkSurfaceFormatKHR> formats; ///< Available surface formats.
    std::vector<VkPresentModeKHR> presentModes; ///< Available presentation modes.
};

/// <summary>
/// Manages the Vulkan logical device and associated resources.
/// </summary>
class VulkanLogicalDevice
{
    VkPhysicalDevice m_vk_physicalDevice; ///< Handle to the Vulkan physical device.
    VkDevice m_vk_logicalDevice; ///< Handle to the Vulkan logical device.
    VkSurfaceKHR m_surface; ///< Vulkan surface handle.
    VkQueue m_graphicsQueue; ///< Handle to the graphics queue.
    VkQueue m_presentQueue; ///< Handle to the presentation queue.
    QueueFamilyIndices m_queueFamilyIndices; ///< Queue family indices used by the device.
    SwapChainSupportDetails m_swapChainSupportDetails; ///< Swap chain support details.

    /// <summary>
    /// Required device extensions.
    /// </summary>
    inline static const std::vector<const char*> deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

public:
    /// <summary>
    /// Constructs a Vulkan logical device.
    /// </summary>
    /// <param name="instance">The Vulkan instance.</param>
    /// <param name="surface">The Vulkan rendering surface.</param>
    /// <exception cref="std::runtime_error">Thrown if device selection or logical device creation fails.</exception>
    VulkanLogicalDevice(VkInstance instance, VkSurfaceKHR surface);

    /// <summary>
    /// Destroys the Vulkan logical device.
    /// </summary>
    ~VulkanLogicalDevice();

    /// <summary>
    /// Retrieves the graphics queue.
    /// </summary>
    /// <returns>Handle to the graphics queue.</returns>
    VkQueue getGraphicsQueue() const { return m_graphicsQueue; }

    /// <summary>
    /// Retrieves the presentation queue.
    /// </summary>
    /// <returns>Handle to the presentation queue.</returns>
    VkQueue getPresentQueue() const { return m_presentQueue; }

    /// <summary>
    /// Retrieves the selected Vulkan physical device.
    /// </summary>
    /// <returns>Handle to the physical device.</returns>
    VkPhysicalDevice getPhysicalDevice() const { return m_vk_physicalDevice; }

    /// <summary>
    /// Retrieves the Vulkan logical device.
    /// </summary>
    /// <returns>Handle to the logical device.</returns>
    VkDevice getLogicalDevice() const { return m_vk_logicalDevice; }

    /// <summary>
    /// Retrieves the queue family indices for the device.
    /// </summary>
    /// <returns>Queue family indices.</returns>
    QueueFamilyIndices getDeviceFamilyIndices() { return findQueueFamilies(m_vk_physicalDevice, m_surface); }

    /// <summary>
    /// Retrieves swap chain support details for the device.
    /// </summary>
    /// <returns>Swap chain support details.</returns>
    SwapChainSupportDetails getDeviceSwapChainSupportDetails() { return querySwapChainSupport(m_vk_physicalDevice, m_surface); }

    /// <summary>
    /// Waits for the device to become idle.
    /// </summary>
    void deviceWaitIdle() const { vkDeviceWaitIdle(m_vk_logicalDevice); }

private:
    /// <summary>
    /// Selects the best available physical device.
    /// </summary>
    /// <param name="instance">The Vulkan instance.</param>
    /// <param name="surface">The Vulkan rendering surface.</param>
    /// <exception cref="std::runtime_error">Thrown if no suitable device is found.</exception>
    void selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

    /// <summary>
    /// Checks whether a given physical device is suitable for use.
    /// </summary>
    /// <param name="physicalDevice">The Vulkan physical device.</param>
    /// <param name="surface">The Vulkan rendering surface.</param>
    /// <returns>True if the device meets all requirements.</returns>
    bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    /// <summary>
    /// Checks if the physical device supports the required extensions.
    /// </summary>
    /// <param name="physicalDevice">The Vulkan physical device.</param>
    /// <returns>True if all required extensions are supported.</returns>
    bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

    /// <summary>
    /// Finds queue families for a given physical device.
    /// </summary>
    /// <param name="physicalDevice">The Vulkan physical device.</param>
    /// <param name="surface">The Vulkan rendering surface.</param>
    /// <returns>Queue family indices.</returns>
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    /// <summary>
    /// Queries swap chain support details for a given physical device.
    /// </summary>
    /// <param name="physicalDevice">The Vulkan physical device.</param>
    /// <param name="surface">The Vulkan rendering surface.</param>
    /// <returns>Swap chain support details.</returns>
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    /// <summary>
    /// Creates the Vulkan logical device.
    /// </summary>
    /// <exception cref="std::runtime_error">Thrown if logical device creation fails.</exception>
    void createLogicalDevice();
};
