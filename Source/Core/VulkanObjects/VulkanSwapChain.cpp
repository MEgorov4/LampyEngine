#include "VulkanSwapChain.h"
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <iostream>
#include "../VulkanApplicationConfig.h"
#include "../Logger/Logger.h"
#include "format"

VulkanSwapChain::~VulkanSwapChain()
{
    cleanupSwapChain();
}

void VulkanSwapChain::recreateSwapChain()
{
    cleanupSwapChain();
    buildSwapChain();
}

void VulkanSwapChain::cleanupSwapChain()
{
    LOG_INFO("VulkanSwapChain: Start cleanup swapchain");
    for (size_t i = 0; i < m_vk_imageViews.size(); i++)
    {
        vkDestroyImageView(m_vk_device, m_vk_imageViews[i], nullptr);
        LOG_INFO("VulkanSwapChain: Image view destroyed: " + std::format("{}", i));
    }
    m_vk_imageViews.clear();
    vkDestroySwapchainKHR(m_vk_device, m_vk_swapChain, nullptr);
    LOG_INFO("VulkanSwapChain: Swapchain destroyed");
}

void VulkanSwapChain::buildSwapChain()
{
    LOG_INFO("VulkanSwapChain: Start create swapchain");

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(m_swapChainSupportDetails.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(m_swapChainSupportDetails.presentModes);
    VkExtent2D extent = chooseSwapExtent(m_vk_extent);

    uint32_t imageCount = m_swapChainSupportDetails.capabilities.minImageCount + 1;
    if (m_swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > m_swapChainSupportDetails.capabilities.maxImageCount) {
        imageCount = m_swapChainSupportDetails.capabilities.maxImageCount;
    }
    else
    {
        if (imageCount >= VulkanApplicationConfig::getInstance().getMaxFramesInFlight())
        {
            //imageCount = VulkanApplicationConfig::getInstance().getMaxFramesInFlight();
            imageCount = 2;
        }
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_vk_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = { m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.presentFamily.value() };

    if (m_queueFamilyIndices.graphicsFamily != m_queueFamilyIndices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = m_swapChainSupportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_vk_device, &createInfo, nullptr, &m_vk_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }
    LOG_INFO("VulkanSwapChain: Swapchain created");

    vkGetSwapchainImagesKHR(m_vk_device, m_vk_swapChain, &imageCount, nullptr);
    m_vk_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_vk_device, m_vk_swapChain, &imageCount, m_vk_images.data());

    LOG_INFO("VulkanSwapChain: Swapchain images count = " + std::format("{}", m_vk_images.size()));

    m_vk_imageFormat = surfaceFormat;
    m_vk_extent = extent;

    createImageViews();
}

void VulkanSwapChain::createImageViews()
{
    m_vk_imageViews.resize(m_vk_images.size());

    for (size_t i = 0; i < m_vk_images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_vk_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_vk_imageFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        
        if (vkCreateImageView(m_vk_device, &createInfo, nullptr, &m_vk_imageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
        LOG_INFO("VulkanSwapChain: Image view created");
    }
}

VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::chooseSwapExtent(VkExtent2D actualExtent)
{
    const VkSurfaceCapabilitiesKHR& capabilities = m_swapChainSupportDetails.capabilities;

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        
        m_vk_extent = actualExtent;

        return actualExtent;
    }
}


