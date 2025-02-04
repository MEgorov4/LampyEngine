#pragma once 
#include <vulkan/vulkan.h>
#include <vector>

/// <summary>
/// Manages Vulkan command buffers, which are used for recording GPU commands.
/// </summary>
class VulkanCommandBuffers
{
    std::vector<VkCommandBuffer> m_vk_commandBuffers; ///< List of Vulkan command buffers.

public:
    /// <summary>
    /// Constructs Vulkan command buffers using the provided command pool and image count.
    /// </summary>
    /// <param name="device">The Vulkan logical device.</param>
    /// <param name="commandPool">The Vulkan command pool from which command buffers are allocated.</param>
    /// <param name="imageCount">The number of command buffers to allocate (usually equal to the number of swap chain images).</param>
    /// <exception cref="std::runtime_error">Thrown if command buffer allocation fails.</exception>
    VulkanCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t imageCount);

    /// <summary>
    /// Retrieves the allocated Vulkan command buffers.
    /// </summary>
    /// <returns>Const reference to the vector of Vulkan command buffers.</returns>
    const std::vector<VkCommandBuffer>& getCommandBuffers() const { return m_vk_commandBuffers; }
};
