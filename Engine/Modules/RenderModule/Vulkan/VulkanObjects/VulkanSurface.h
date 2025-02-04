#pragma once
#include "vulkan/vulkan.h"

/// <summary>
/// Manages a Vulkan surface, which represents a platform-specific window rendering target.
/// </summary>
class VulkanSurface
{
    VkInstance m_vk_instance; ///< Handle to the Vulkan instance.
    VkSurfaceKHR m_vk_surface; ///< Handle to the Vulkan surface.

public:
    /// <summary>
    /// Constructs a Vulkan surface using the given Vulkan instance and surface handle.
    /// </summary>
    /// <param name="instance">The Vulkan instance associated with the surface.</param>
    /// <param name="surface">The Vulkan surface handle.</param>
    VulkanSurface(VkInstance instance, VkSurfaceKHR surface);

    /// <summary>
    /// Destroys the Vulkan surface.
    /// </summary>
    ~VulkanSurface();

    /// <summary>
    /// Retrieves the Vulkan surface handle.
    /// </summary>
    /// <returns>The Vulkan surface handle.</returns>
    VkSurfaceKHR getSurface() const { return m_vk_surface; }
};
