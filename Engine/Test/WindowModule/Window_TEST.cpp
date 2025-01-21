#include <gtest/gtest.h>
#include <vulkan/vulkan.h>
#include "../../Modules/WindowModule/Window.h"


TEST(WindowTest, CreateWindow)
{
    int width = 800;
    int height = 600;
    const char* title = "Test Window";

    Window window(width, height, title);

    EXPECT_FALSE(window.shouldClose()) << "Window should do not closed on start";

    auto extensions = window.getRequiredInstanceExtensions();
    EXPECT_FALSE(extensions.empty()) << "Extensions cannot be empty";
}

TEST(WindowTest, ResizeWindow) 
{
    int width = 800;
    int height = 600;
    const char* title = "Test Resizable Window";

    Window window(width, height, title);

    glfwSetWindowSize(window.getGLFWWindow(), 1024, 768);

    EXPECT_TRUE(window.wasResized()) << "Resized flag is not placed";

    window.resetResizedFlag();
    EXPECT_FALSE(window.wasResized()) << "Flag is not reseted";
}

TEST(WindowTest, CreateSurface)
{
    int width = 800;
    int height = 600;
    const char* title = "Test Surface Window";

    Window window(width, height, title);

    EXPECT_TRUE(window.glfwWindowIsValid()) << "glfw window is not valid";

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "LampyEngine";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "No Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    auto extensions = window.getRequiredInstanceExtensions();
    EXPECT_FALSE(extensions.empty()) << "Extensions list empty";

    VkInstance instance;
    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) 
    {
        FAIL() << "Не удалось создать Vulkan Instance";
    }

    VkSurfaceKHR surface = window.getWindowSurface(instance);

    EXPECT_NE(surface, VK_NULL_HANDLE) << "Surface is not created";

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}