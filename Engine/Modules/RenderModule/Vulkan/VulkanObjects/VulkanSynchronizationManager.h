#pragma once

#include <vulkan/vulkan.h>
#include <vector>

/// <summary>
/// Manages Vulkan synchronization objects, including semaphores and fences, to synchronize frame rendering.
/// </summary>
class VulkanSynchronizationManager {
    VkDevice m_device; ///< Handle to the Vulkan logical device.
    std::vector<VkSemaphore> m_imageAvailableSemaphores; ///< Semaphores to signal when an image is available for rendering.
    std::vector<VkSemaphore> m_renderFinishedSemaphores; ///< Semaphores to signal when rendering is finished.
    std::vector<VkFence> m_inFlightFences; ///< Fences to ensure frame synchronization.

public:
    /// <summary>
    /// Constructs a synchronization manager for handling Vulkan semaphores and fences.
    /// </summary>
    /// <param name="device">The Vulkan logical device.</param>
    /// <param name="maxFramesInFlight">Maximum number of frames in flight.</param>
    /// <exception cref="std::runtime_error">Thrown if synchronization objects cannot be created.</exception>
    VulkanSynchronizationManager(VkDevice device, uint32_t maxFramesInFlight);

    /// <summary>
    /// Destroys all Vulkan synchronization objects.
    /// </summary>
    ~VulkanSynchronizationManager();

    /// <summary>
    /// Retrieves the semaphore that signals when an image is available for rendering.
    /// </summary>
    /// <param name="currentFrame">The index of the current frame.</param>
    /// <returns>Reference to the image available semaphore.</returns>
    VkSemaphore& getImageAvailableSemaphore(uint32_t currentFrame) {
        return m_imageAvailableSemaphores[currentFrame];
    }

    /// <summary>
    /// Retrieves the semaphore that signals when rendering is finished.
    /// </summary>
    /// <param name="currentFrame">The index of the current frame.</param>
    /// <returns>Reference to the render finished semaphore.</returns>
    VkSemaphore& getRenderFinishedSemaphore(uint32_t currentFrame) {
        return m_renderFinishedSemaphores[currentFrame];
    }

    /// <summary>
    /// Retrieves the fence used to synchronize frame execution.
    /// </summary>
    /// <param name="currentFrame">The index of the current frame.</param>
    /// <returns>Reference to the in-flight fence.</returns>
    VkFence& getInFlightFence(uint32_t currentFrame) {
        return m_inFlightFences[currentFrame];
    }
};

