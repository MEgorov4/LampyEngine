#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanLogicalDevice.h"
class VulkanSwapChain
{
    VkSwapchainKHR m_vk_swapChain;
    VkDevice m_vk_device;

    VkSurfaceKHR m_vk_surface;
    VkSurfaceFormatKHR m_vk_imageFormat;

    VkExtent2D m_vk_extent;

    std::vector<VkImage> m_vk_images;
    std::vector<VkImageView> m_vk_imageViews;

    SwapChainSupportDetails m_swapChainSupportDetails;
    QueueFamilyIndices m_queueFamilyIndices;
public:
    VulkanSwapChain(VkDevice device, VkSurfaceKHR surface, VkExtent2D extent, SwapChainSupportDetails swapChainSupportDetails, QueueFamilyIndices queueFamilyIndices) :
        m_vk_device(device),
        m_vk_surface(surface),
        m_vk_extent(extent),
        m_swapChainSupportDetails(swapChainSupportDetails),
        m_queueFamilyIndices(queueFamilyIndices) 
    {
        buildSwapChain();
    };

	~VulkanSwapChain();

    void recreateSwapChain();
    
    VkSwapchainKHR getSwapChain() const { return m_vk_swapChain; };

    VkSurfaceFormatKHR getSurfaceFormat() const { return m_vk_imageFormat; }
    VkExtent2D getExtent() const { return m_vk_extent; }
    void setExtent(const VkExtent2D& extent) { m_vk_extent = extent; }

    const std::vector<VkImageView>& getImageViews() const { return m_vk_imageViews; }
    
private:
    void buildSwapChain();
    void createImageViews();

    void cleanupSwapChain();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent( VkExtent2D extent);
    
};