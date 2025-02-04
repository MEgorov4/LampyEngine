#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

/// <summary>
/// Manages a Vulkan instance, including validation layers and debug messaging.
/// </summary>
class VulkanInstance
{
    VkInstance m_vk_instance;                    ///< Handle to the Vulkan instance.
    VkDebugUtilsMessengerEXT m_debugMessenger;   ///< Handle to the Vulkan debug messenger.
    bool m_enableValidationLayers;               ///< Flag to enable or disable validation layers.
    std::vector<const char*> m_validationLayers; ///< List of validation layers to be enabled.

public:
    /// <summary>
    /// Constructs a Vulkan instance with the specified extensions and validation layers.
    /// </summary>
    /// <param name="requiredExtensions">A list of required Vulkan instance extensions.</param>
    /// <param name="enableValidationLayers">Flag to enable Vulkan validation layers (default: false).</param>
    /// <exception cref="std::runtime_error">Thrown if instance creation fails or validation layers are not available.</exception>
    explicit VulkanInstance(std::vector<const char*> requiredExtensions, bool enableValidationLayers = false);

    /// <summary>
    /// Destroys the Vulkan instance and cleans up resources.
    /// </summary>
    ~VulkanInstance();

    /// <summary>
    /// Retrieves the Vulkan instance handle.
    /// </summary>
    /// <returns>Handle to the Vulkan instance.</returns>
    VkInstance getInstance() const { return m_vk_instance; }

private:
    /// <summary>
    /// Checks if the requested validation layers are supported by the Vulkan implementation.
    /// </summary>
    /// <returns>True if all requested validation layers are available, otherwise false.</returns>
    bool checkValidationLayerSupport();

    /// <summary>
    /// Sets up the Vulkan debug messenger for validation and debugging purposes.
    /// </summary>
    /// <exception cref="std::runtime_error">Thrown if the debug messenger cannot be created.</exception>
    void setupDebugMessenger();

    /// <summary>
    /// Creates the Vulkan debug messenger extension if available.
    /// </summary>
    /// <param name="instance">Vulkan instance.</param>
    /// <param name="pCreateInfo">Pointer to debug messenger creation info.</param>
    /// <param name="pAllocator">Custom memory allocator (can be nullptr).</param>
    /// <param name="pDebugMessenger">Pointer to store the created debug messenger.</param>
    /// <returns>VkResult indicating success or failure of the operation.</returns>
    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

    /// <summary>
    /// Destroys the Vulkan debug messenger extension if available.
    /// </summary>
    /// <param name="instance">Vulkan instance.</param>
    /// <param name="debugMessenger">Debug messenger handle.</param>
    /// <param name="pAllocator">Custom memory allocator (can be nullptr).</param>
    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);

    /// <summary>
    /// Callback function for Vulkan debug messages.
    /// </summary>
    /// <param name="messageSeverity">Severity level of the debug message.</param>
    /// <param name="messageType">Type of debug message.</param>
    /// <param name="pCallbackData">Pointer to the debug message data.</param>
    /// <param name="pUserData">User-defined data (unused).</param>
    /// <returns>VkBool32 (VK_FALSE to indicate the message has been handled).</returns>
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};

