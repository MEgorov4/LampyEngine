#include "VulkanRenderer.h"
#include "../VulkanApplicationConfig.h"
#include <stdexcept>
#include <iostream>
#include "../VulkanApplication.h"

VulkanRenderer::VulkanRenderer(
    VulkanApplication* application,
    VkDevice device,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VulkanSwapChain* swapChain,
    VulkanCommandBuffers* commandBuffers,
    VulkanRenderPass* renderPass,
    VulkanGraphicsPipeline* graphicsPipeline,
    const std::vector<VkFramebuffer>& framebuffers
)
    : m_application(application),
    m_device(device),
    m_graphicsQueue(graphicsQueue),
    m_presentQueue(presentQueue),
    m_swapChain(swapChain),
    m_commandBuffers(commandBuffers),
    m_renderPass(renderPass),
    m_graphicsPipeline(graphicsPipeline),
    m_framebuffers(framebuffers),
    m_syncManager(device, VulkanApplicationConfig::getInstance().getMaxFramesInFlight())
{
    // Здесь уже ничего не двигаем и не удаляем,
    // просто сохраняем сырые указатели
}

VulkanRenderer::~VulkanRenderer() {
    vkDeviceWaitIdle(m_device);
}

void VulkanRenderer::drawFrame() {
    vkWaitForFences(m_device, 1, &m_syncManager.getInFlightFence(m_currentFrame), VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_device,
        m_swapChain->getSwapChain(),
        UINT64_MAX,
        m_syncManager.getImageAvailableSemaphore(m_currentFrame),
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        m_application->recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(m_device, 1, &m_syncManager.getInFlightFence(m_currentFrame));


    vkResetCommandBuffer(m_commandBuffers->getCommandBuffers()[m_currentFrame], 0);

    std::cout << "Command Buffers Size: " << m_commandBuffers->getCommandBuffers().size() << std::endl;
    std::cout << "Framebuffers Size: " << m_framebuffers.size() << std::endl;
    std::cout << "Current Frame: " << m_currentFrame << ", Image Index: " << imageIndex << std::endl;
    
    if (m_framebuffers[imageIndex] == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Framebuffer is null!");
    }
    recordCommandBuffer(m_commandBuffers->getCommandBuffers()[m_currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_syncManager.getImageAvailableSemaphore(m_currentFrame) };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers->getCommandBuffers()[m_currentFrame];
    
    std::cout << "Pre submit command buffer address: " << m_commandBuffers->getCommandBuffers()[m_currentFrame] << std::endl;

    VkSemaphore signalSemaphores[] = { m_syncManager.getRenderFinishedSemaphore(m_currentFrame) };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_syncManager.getInFlightFence(m_currentFrame)) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_swapChain->getSwapChain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_swapChain->getExtent().width == 0 || m_swapChain->getExtent().height == 0) {
        m_application->recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % VulkanApplicationConfig::getInstance().getMaxFramesInFlight();
}

void VulkanRenderer::updateVertexBuffers(const std::vector<std::unique_ptr<VulkanVertexBuffer>>& vertexBuffers)
{
    m_vertexBuffers.clear();
    m_vertexBuffers.reserve(vertexBuffers.size());
    for (auto& vbPtr : vertexBuffers) 
    {
        m_vertexBuffers.push_back(vbPtr.get()); // сохраняем просто указатель
    }
}


void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass->getRenderPass();
    renderPassInfo.framebuffer = m_framebuffers[imageIndex];

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapChain->getExtent();

    VkClearValue clearColor = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->getPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapChain->getExtent().width;
    viewport.height = (float)m_swapChain->getExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapChain->getExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { m_vertexBuffers[0]->getBuffer()};
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}
