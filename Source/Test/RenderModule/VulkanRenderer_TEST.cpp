#include <gtest/gtest.h>

#include "../../Engine/VulkanApplicationConfig.h"
#include "../../WindowModule/Window.h"

#include "../../RenderModule/Vulkan/VulkanObjects/VulkanInstance.h"
#include "../../RenderModule/Vulkan/VulkanObjects/VulkanSurface.h"
#include "../../RenderModule/Vulkan/VulkanObjects/VulkanLogicalDevice.h"
#include "../../RenderModule/Vulkan/VulkanObjects/VulkanSwapChain.h"

#include "../../RenderModule/Vulkan/VulkanObjects/VulkanCommandPool.h"
#include "../../RenderModule/Vulkan/VulkanObjects/VulkanRenderPass.h"
#include "../../RenderModule/Vulkan/VulkanObjects/VulkanFrameBuffers.h"
#include "../../RenderModule/Vulkan/VulkanObjects/VulkanCommandBuffers.h"
#include "../../RenderModule/Vulkan/VulkanObjects/VulkanSynchronizationManager.h"


TEST(VulkanObjectsTest, SetupVulkanObjects)
{
	Window* window = new Window(800, 600, "ConstructVulkanRendererTest");


	EXPECT_FALSE(window->shouldClose()) << "Window cannot be close on start";

	std::vector<const char*> extensions = window->getRequiredInstanceExtensions();
	EXPECT_FALSE(extensions.empty()) << "Window extensions cannot be empty";

	VulkanInstance* instance = new VulkanInstance(extensions, false);

	EXPECT_NE(instance->getInstance(), VK_NULL_HANDLE) << "Failed to create vulkan instance";
	
	VkSurfaceKHR surfaceKHR = window->getWindowSurface(instance->getInstance());

	EXPECT_NE(surfaceKHR, VK_NULL_HANDLE) << "Failed to get vulkan surface from glfw window";

	VulkanSurface* surface = new VulkanSurface(instance->getInstance(), surfaceKHR);

	EXPECT_NE(surface->getSurface(), VK_NULL_HANDLE) <<  "Failed to init vulkan surface";

	VulkanLogicalDevice* logicalDevice = new VulkanLogicalDevice(instance->getInstance(), surface->getSurface());

	EXPECT_NE(logicalDevice->getPhysicalDevice(), VK_NULL_HANDLE) << "Physical device non valid";
	EXPECT_NE(logicalDevice->getLogicalDevice(), VK_NULL_HANDLE) << "Logical device non valid";
	EXPECT_NE(logicalDevice->getGraphicsQueue(), VK_NULL_HANDLE) << "Graphics queue non valid";
	EXPECT_NE(logicalDevice->getPresentQueue(), VK_NULL_HANDLE) << "Present queue non valid";
	
	VulkanSwapChain* swapChain = new VulkanSwapChain(logicalDevice->getLogicalDevice(),
		surface->getSurface(),
		window->getExtent(),
		logicalDevice->getDeviceSwapChainSupportDetails(),
		logicalDevice->getDeviceFamilyIndices());

	
	EXPECT_NE(swapChain->getSwapChain(), VK_NULL_HANDLE) << "SwapChain non valid";

	std::vector<VkImageView> imageViews = swapChain->getImageViews();

	EXPECT_EQ(imageViews.size(), EngineApplicationConfig::getInstance().getMaxFramesInFlight()) << "Image views wrong count";
	
	for (VkImageView view : imageViews)
	{
		EXPECT_NE(view, VK_NULL_HANDLE) << "Image view non valid";
	}

	VulkanCommandPool* commandPool = new VulkanCommandPool(logicalDevice->getLogicalDevice(), 
														   logicalDevice->getDeviceFamilyIndices().graphicsFamily.value());

	EXPECT_NE(commandPool->getCommandPool(), VK_NULL_HANDLE) << "Command pool non valid";

	
	VulkanRenderPass* renderPass = new VulkanRenderPass(logicalDevice->getLogicalDevice(),
														swapChain->getSurfaceFormat().format);
	
	EXPECT_NE(renderPass->getRenderPass(), VK_NULL_HANDLE) << "RenderPass non valid";

	VulkanFramebuffers* framebuffers = new VulkanFramebuffers(logicalDevice->getLogicalDevice(), renderPass->getRenderPass(), swapChain->getExtent(), swapChain->getImageViews());

	std::vector<VkFramebuffer> v_framebuffers = framebuffers->getFramebuffers();

	EXPECT_EQ(v_framebuffers.size(), EngineApplicationConfig::getInstance().getMaxFramesInFlight()) << "Framebuffers wrong count";

	for (VkFramebuffer framebuffer : v_framebuffers)
	{
		EXPECT_NE(framebuffer, VK_NULL_HANDLE) << "Framebuffer non valid";
	}

	VulkanCommandBuffers* commandBuffers = new VulkanCommandBuffers(logicalDevice->getLogicalDevice(), 
																	commandPool->getCommandPool(), 
																	EngineApplicationConfig::getInstance().getMaxFramesInFlight());
	std::vector<VkCommandBuffer> v_commandBuffers = commandBuffers->getCommandBuffers();


	EXPECT_EQ(v_commandBuffers.size(), EngineApplicationConfig::getInstance().getMaxFramesInFlight()) << "Command wrong count";

	for (VkCommandBuffer commandBuffer : v_commandBuffers)
	{
		EXPECT_NE(commandBuffer, VK_NULL_HANDLE) << "Command buffer non valid";
	}

	VulkanSynchronizationManager* syncManager = new VulkanSynchronizationManager(logicalDevice->getLogicalDevice(),
																				 EngineApplicationConfig::getInstance().getMaxFramesInFlight());
	
	for (uint32_t i = 0; i < EngineApplicationConfig::getInstance().getMaxFramesInFlight(); i++)
	{
		EXPECT_NE(syncManager->getInFlightFence(i), VK_NULL_HANDLE) << "In flight fence non valid";
		EXPECT_NE(syncManager->getImageAvailableSemaphore(i), VK_NULL_HANDLE) << "Image avaliable semaphore non valid";
		EXPECT_NE(syncManager->getRenderFinishedSemaphore(i), VK_NULL_HANDLE) << "Render finished semaphore non valid";
	}

	delete syncManager;
	delete commandBuffers;
	delete framebuffers;
	delete renderPass;
	delete commandPool;
	delete swapChain;
	delete logicalDevice;
	delete surface;
	delete instance;
	delete window;
}