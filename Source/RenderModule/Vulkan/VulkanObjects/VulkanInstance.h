#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanInstance
{
    VkInstance m_vk_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    bool m_enableValidationLayers;
    std::vector<const char*> m_validationLayers;

public:
    VulkanInstance(std::vector<const char*> requiredExtensions, bool enableValidationLayers = false);
    ~VulkanInstance();

    VkInstance getInstance() const { return m_vk_instance; }

private:
    bool checkValidationLayerSupport();
    void setupDebugMessenger();
    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};
