#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

#include "VulkanSwapChain.h"
#include "VulkanCommandBuffers.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanSynchronizationManager.h"
#include "VulkanVertexBuffer.h"

class VulkanApplication;
class VulkanSwapChain;
class VulkanCommandBuffers;
class VulkanRenderPass;

class VulkanRenderer {
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    VulkanApplication* m_application;

    VulkanSwapChain* m_swapChain;
    VulkanCommandBuffers* m_commandBuffers;
    VulkanRenderPass* m_renderPass;
    VulkanGraphicsPipeline* m_graphicsPipeline;

    std::vector<VulkanVertexBuffer*> m_vertexBuffers;

    std::vector<VkFramebuffer> m_framebuffers;

    VulkanSynchronizationManager m_syncManager;

    uint32_t m_currentFrame = 0;

public:
    VulkanRenderer(
        VulkanApplication* application,
        VkDevice device,
        VkQueue graphicsQueue,
        VkQueue presentQueue,
        VulkanSwapChain* swapChain,            
        VulkanCommandBuffers* commandBuffers,  
        VulkanRenderPass* renderPass,         
        VulkanGraphicsPipeline* graphicsPipeline,
        const std::vector<VkFramebuffer>& framebuffers
    );
    ~VulkanRenderer();

    void drawFrame();
    void updateVertexBuffers(const std::vector<std::unique_ptr<VulkanVertexBuffer>>& vertexBuffers);
private:
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
};

