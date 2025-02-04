#pragma once
#include <vulkan/vulkan.h>
#include <vector>

/// <summary>
/// Manages Vulkan framebuffers, which are used as rendering targets in a render pass.
/// </summary>
class VulkanFramebuffers
{
    std::vector<VkFramebuffer> m_framebuffers; ///< List of Vulkan framebuffers.
    VkDevice m_device; ///< Handle to the Vulkan logical device.

public:
    /// <summary>
    /// Constructs Vulkan framebuffers for the given render pass and image views.
    /// </summary>
    /// <param name="device">The Vulkan logical device.</param>
    /// <param name="renderPass">The Vulkan render pass.</param>
    /// <param name="extent">The dimensions of the framebuffers.</param>
    /// <param name="imageViews">A vector of image views to be attached to the framebuffers.</param>
    /// <exception cref="std::runtime_error">Thrown if framebuffer creation fails.</exception>
    VulkanFramebuffers(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& imageViews);

    /// <summary>
    /// Destroys all Vulkan framebuffers.
    /// </summary>
    ~VulkanFramebuffers();

    /// <summary>
    /// Retrieves the list of Vulkan framebuffers.
    /// </summary>
    /// <returns>Const reference to the vector of Vulkan framebuffers.</returns>
    const std::vector<VkFramebuffer>& getFramebuffers() const { return m_framebuffers; }

private:
    /// <summary>
    /// Recreates all framebuffers by first destroying existing ones and then creating new ones.
    /// </summary>
    /// <param name="renderPass">The Vulkan render pass.</param>
    /// <param name="extent">The dimensions of the framebuffers.</param>
    /// <param name="imageViews">A vector of image views to be attached to the framebuffers.</param>
    void recreateFramebuffers(VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& imageViews);

    /// <summary>
    /// Creates Vulkan framebuffers for the given render pass and image views.
    /// </summary>
    /// <param name="renderPass">The Vulkan render pass.</param>
    /// <param name="extent">The dimensions of the framebuffers.</param>
    /// <param name="imageViews">A vector of image views to be attached to the framebuffers.</param>
    /// <exception cref="std::runtime_error">Thrown if framebuffer creation fails.</exception>
    void createFramebuffers(VkRenderPass renderPass, VkExtent2D extent, std::vector<VkImageView> imageViews);

    /// <summary>
    /// Destroys all existing framebuffers.
    /// </summary>
    void clearFramebuffers();
};
