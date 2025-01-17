#pragma once
#include "vulkan/vulkan.h"
#include "memory"
#include "vector"
#include "ObjectModel/3D/MeshInstance.h"
#include "VulkanObjects/VulkanRenderPass.h"
#include "ObjectModel/Scene.h"

class Window;
class VulkanInstance;
class VulkanSurface;
class VulkanLogicalDevice;
class VulkanSwapChain;
class VulkanGraphicsPipeline;
class VulkanFramebuffers;
class VulkanCommandPool;
class VulkanCommandBuffers;
class VulkanVertexBuffer;
class VulkanSynchronizationManager;
class VulkanRenderer;
class ShaderManager;
class Mesh;

class VulkanApplication
{
	std::unique_ptr<Window> m_window;

	std::unique_ptr<VulkanInstance> m_instance;
	std::unique_ptr<VulkanSurface> m_surface;
	std::unique_ptr<VulkanLogicalDevice> m_logicalDevice;

	std::unique_ptr<VulkanSwapChain> m_swapChain;
	std::unique_ptr<VulkanRenderPass> m_renderPass;
	std::unique_ptr<VulkanFramebuffers> m_framebuffers;

	std::unique_ptr<VulkanCommandPool> m_commandPool;
	std::unique_ptr<VulkanCommandBuffers> m_commandBuffers;

	std::unique_ptr<VulkanSynchronizationManager> m_syncManager;

	MeshInstance* m_mesh;
	Scene* m_scene;

	uint32_t m_currentFrame = 0;
	bool m_framebufferResized = false;
public:
	VulkanApplication();
	VulkanApplication(const VulkanApplication& vapp) = delete;
	~VulkanApplication();
	const VulkanApplication& operator=(const VulkanApplication vapp) = delete;

	void run();
	void initEngine();
	void initWindow();
	void initVulkan();
	void mainLoop();

	void drawFrame();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void createSwapChainAndDependentResources();
	void cleanupSwapChainAndDependentResources();
	void recreateSwapChain();
	
	VulkanLogicalDevice* getLogicalDevice() { return m_logicalDevice.get(); }
	VulkanCommandPool* getCommandPool() { return m_commandPool.get(); }
	VulkanRenderPass* getRenderPass() { return m_renderPass.get(); }
};