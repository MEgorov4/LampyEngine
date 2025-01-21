#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanSynchronizationManager {
    VkDevice m_device;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

public:
    VulkanSynchronizationManager(VkDevice device, uint32_t maxFramesInFlight);
    ~VulkanSynchronizationManager();

    VkSemaphore& getImageAvailableSemaphore(uint32_t currentFrame) {
        return m_imageAvailableSemaphores[currentFrame];
    }

    VkSemaphore& getRenderFinishedSemaphore(uint32_t currentFrame) {
        return m_renderFinishedSemaphores[currentFrame];
    }

    VkFence& getInFlightFence(uint32_t currentFrame) {
        return m_inFlightFences[currentFrame];
    }
};

