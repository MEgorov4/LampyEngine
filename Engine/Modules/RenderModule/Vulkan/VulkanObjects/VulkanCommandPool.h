#pragma once
#include <vulkan/vulkan.h>

/// <summary>
/// Manages a Vulkan command pool, which is used to allocate command buffers.
/// </summary>
class VulkanCommandPool {
    VkCommandPool m_vk_commandPool; ///< Handle to the Vulkan command pool.
    VkDevice m_device; ///< Handle to the Vulkan logical device.

public:
    /// <summary>
    /// Constructs a Vulkan command pool for the given queue family index.
    /// </summary>
    /// <param name="device">The Vulkan logical device.</param>
    /// <param name="queueFamilyIndex">The index of the queue family that will use this command pool.</param>
    /// <exception cref="std::runtime_error">Thrown if command pool creation fails.</exception>
    VulkanCommandPool(VkDevice device, uint32_t queueFamilyIndex);

    /// <summary>
    /// Destroys the Vulkan command pool.
    /// </summary>
    ~VulkanCommandPool();

    /// <summary>
    /// Retrieves the Vulkan command pool handle.
    /// </summary>
    /// <returns>Handle to the Vulkan command pool.</returns>
    VkCommandPool getCommandPool() const { return m_vk_commandPool; }
};
