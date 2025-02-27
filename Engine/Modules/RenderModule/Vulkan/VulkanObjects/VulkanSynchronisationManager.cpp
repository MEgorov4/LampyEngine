#include "VulkanSynchronizationManager.h"
#include <stdexcept>
#include "../../../LoggerModule/Logger.h"

VulkanSynchronizationManager::VulkanSynchronizationManager(VkDevice device, uint32_t maxFramesInFlight)
    : m_device(device)
{
    LOG_INFO("VulkanSynchronizationManager: Start create sync objects");
    m_imageAvailableSemaphores.resize(maxFramesInFlight);
    m_renderFinishedSemaphores.resize(maxFramesInFlight);
    m_inFlightFences.resize(maxFramesInFlight);

    offscreenSemaphores.resize(maxFramesInFlight);
    offscreenFinishedSemaphores.resize(maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &offscreenSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &offscreenFinishedSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
        LOG_INFO("VulkanSynchronizationManager: Semaphore created");
        LOG_INFO("VulkanSynchronizationManager: Semaphore created");
        LOG_INFO("VulkanSynchronizationManager: Fence create");
    }
    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_offscreenRenderFinishedSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

VulkanSynchronizationManager::~VulkanSynchronizationManager() 
{
    for (size_t i = 0; i < m_imageAvailableSemaphores.size(); i++)
    {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        LOG_INFO("VulkanSynchronizationManager: Semaphore destroyed");
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        LOG_INFO("VulkanSynchronizationManager: Semaphore destroyed");
        vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
        LOG_INFO("VulkanSynchronizationManager: Fence destroyed");
        vkDestroySemaphore(m_device, offscreenSemaphores[i], nullptr);
        LOG_INFO("VulkanSynchronizationManager: Semaphore destroyed");
        vkDestroySemaphore(m_device, offscreenFinishedSemaphores[i], nullptr);
    }
    LOG_INFO("VulkanSynchronizationManager: Semaphore destroyed");
    vkDestroySemaphore(m_device, m_offscreenRenderFinishedSemaphore, nullptr);
}
