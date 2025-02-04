#pragma once 
#include <vulkan/vulkan.h>

/// <summary>
/// Manages a Vulkan render pass, defining how framebuffer attachments are used during rendering.
/// </summary>
class VulkanRenderPass {
    VkRenderPass m_vk_renderPass; ///< Handle to the Vulkan render pass.
    VkDevice m_vk_device; ///< Handle to the Vulkan logical device.

public:
    /// <summary>
    /// Constructs a Vulkan render pass with the given device and image format.
    /// </summary>
    /// <param name="device">The Vulkan logical device.</param>
    /// <param name="imageFormat">The format of the images used in the render pass.</param>
    /// <exception cref="std::runtime_error">Thrown if the render pass creation fails.</exception>
    VulkanRenderPass(VkDevice device, VkFormat imageFormat);

    /// <summary>
    /// Destroys the Vulkan render pass.
    /// </summary>
    ~VulkanRenderPass();

    /// <summary>
    /// Retrieves the Vulkan render pass handle.
    /// </summary>
    /// <returns>Handle to the Vulkan render pass.</returns>
    VkRenderPass getRenderPass() const { return m_vk_renderPass; }
};
