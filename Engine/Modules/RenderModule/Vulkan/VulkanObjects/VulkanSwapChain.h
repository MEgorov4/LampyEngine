#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanLogicalDevice.h"

/// <summary>
/// Manages the Vulkan swap chain, which handles image presentation and synchronization.
/// </summary>
class VulkanSwapChain
{
    VkSwapchainKHR m_vk_swapChain; ///< Handle to the Vulkan swap chain.
    VkDevice m_vk_device; ///< Vulkan logical device handle.
    VkSurfaceKHR m_vk_surface; ///< Vulkan surface handle.
    VkSurfaceFormatKHR m_vk_imageFormat; ///< Format of the swap chain images.
    VkExtent2D m_vk_extent; ///< Dimensions of the swap chain images.
    std::vector<VkImage> m_vk_images; ///< List of swap chain images.
    std::vector<VkImageView> m_vk_imageViews; ///< List of image views for swap chain images.
    SwapChainSupportDetails m_swapChainSupportDetails; ///< Details of swap chain support.
    QueueFamilyIndices m_queueFamilyIndices; ///< Queue family indices for graphics and presentation.
    VkFormat m_swapChainImageFormat;

public:
    /// <summary>
    /// Constructs a Vulkan swap chain.
    /// </summary>
    /// <param name="device">Vulkan logical device.</param>
    /// <param name="surface">Vulkan rendering surface.</param>
    /// <param name="extent">Desired swap chain extent.</param>
    /// <param name="swapChainSupportDetails">Details of swap chain support capabilities.</param>
    /// <param name="queueFamilyIndices">Queue family indices for the device.</param>
    VulkanSwapChain(VkDevice device, VkSurfaceKHR surface, VkExtent2D extent,
        SwapChainSupportDetails swapChainSupportDetails, QueueFamilyIndices queueFamilyIndices);

    /// <summary>
    /// Destroys the swap chain and its associated resources.
    /// </summary>
    ~VulkanSwapChain();

    /// <summary>
    /// Recreates the swap chain in case of window resizing or other changes.
    /// </summary>
    void recreateSwapChain();

    /// <summary>
    /// Retrieves the Vulkan swap chain handle.
    /// </summary>
    /// <returns>Handle to the Vulkan swap chain.</returns>
    VkSwapchainKHR getSwapChain() const { return m_vk_swapChain; }

    /// <summary>
    /// Retrieves the surface format of the swap chain images.
    /// </summary>
    /// <returns>Surface format used by the swap chain.</returns>
    VkSurfaceFormatKHR getSurfaceFormat() const { return m_vk_imageFormat; }
    VkFormat getFormat() const { return m_swapChainImageFormat; }

    /// <summary>
    /// Retrieves the swap chain image extent (dimensions).
    /// </summary>
    /// <returns>Extent (width and height) of swap chain images.</returns>
    VkExtent2D getExtent() const { return m_vk_extent; }

    /// <summary>
    /// Sets the swap chain extent (dimensions).
    /// </summary>
    /// <param name="extent">New extent to set.</param>
    void setExtent(const VkExtent2D& extent) { m_vk_extent = extent; }

    /// <summary>
    /// Retrieves the list of image views associated with the swap chain images.
    /// </summary>
    /// <returns>Reference to a vector of image views.</returns>
    const std::vector<VkImageView>& getImageViews() const { return m_vk_imageViews; }

private:
    /// <summary>
    /// Creates the Vulkan swap chain.
    /// </summary>
    void buildSwapChain();

    /// <summary>
    /// Creates image views for swap chain images.
    /// </summary>
    void createImageViews();

    /// <summary>
    /// Cleans up swap chain resources.
    /// </summary>
    void cleanupSwapChain();

    /// <summary>
    /// Chooses the best surface format from available options.
    /// </summary>
    /// <param name="availableFormats">List of available surface formats.</param>
    /// <returns>Chosen surface format.</returns>
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /// <summary>
    /// Chooses the best presentation mode for the swap chain.
    /// </summary>
    /// <param name="availablePresentModes">List of available present modes.</param>
    /// <returns>Chosen present mode.</returns>
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    /// <summary>
    /// Determines the swap chain extent based on window size constraints.
    /// </summary>
    /// <param name="actualExtent">Requested swap chain extent.</param>
    /// <returns>Final swap chain extent.</returns>
    VkExtent2D chooseSwapExtent(VkExtent2D actualExtent);
};
