#include "VulkanFramebuffers.h"
#include <stdexcept>
#include <iostream>
#include "../../../LoggerModule/Logger.h"
#include "format"

VulkanFramebuffers::VulkanFramebuffers(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& imageViews) : m_device(device)
{
    recreateFramebuffers(renderPass, extent, imageViews);
}

VulkanFramebuffers::~VulkanFramebuffers()
{
    clearFramebuffers();
}

void VulkanFramebuffers::recreateFramebuffers(VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& imageViews)
{
    clearFramebuffers();
    createFramebuffers(renderPass, extent, imageViews);
}

void VulkanFramebuffers::createFramebuffers(VkRenderPass renderPass, VkExtent2D extent, std::vector<VkImageView> imageViews)
{
    LOG_INFO("VulkanFramebuffers: Start create framebuffers");

    m_framebuffers.resize(imageViews.size());

    for (size_t i = 0; i < imageViews.size(); i++) {
        VkImageView attachments[] = {
            imageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }

        LOG_INFO("VulkanFramebuffers: Framebuffer created " + std::to_string(reinterpret_cast<uintptr_t>(m_framebuffers[i])));
    }
}

void VulkanFramebuffers::clearFramebuffers()
{
    for (size_t i = 0; i < m_framebuffers.size(); i++)
    {
        vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
        LOG_INFO("VulkanFramebuffers: Framebuffer destroyed " + std::format("{}", i));
    }
    m_framebuffers.clear();
}
