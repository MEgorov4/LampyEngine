#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

#include "../IRenderer.h"
#include "VulkanObjects/VulkanGraphicsPipeline.h"
class Window;
class VulkanInstance;
class VulkanSurface;
class VulkanLogicalDevice;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanFramebuffers;
class VulkanCommandPool;
class VulkanCommandBuffers;
class VulkanSynchronizationManager;

class VulkanPipelineCache;
class VulkanVertexBufferCache;


class VulkanRenderer : public IRenderer
{
	std::unique_ptr<VulkanInstance> m_instance;
	std::unique_ptr<VulkanSurface> m_surface;
	std::unique_ptr<VulkanLogicalDevice> m_logicalDevice;
	std::unique_ptr<VulkanSwapChain> m_swapChain;
	std::unique_ptr<VulkanRenderPass> m_renderPass;
	std::unique_ptr<VulkanFramebuffers> m_framebuffers;
	std::unique_ptr<VulkanCommandPool> m_commandPool;
	std::unique_ptr<VulkanCommandBuffers> m_commandBuffers;
	std::unique_ptr<VulkanSynchronizationManager> m_syncManager;

    std::unique_ptr<VulkanPipelineCache> m_pipelineCache;
	std::unique_ptr<VulkanVertexBufferCache> m_vertexBufferCache;

    uint32_t m_currentFrame = 0;

	Window* m_window;
public:
    explicit VulkanRenderer(Window* window);
    ~VulkanRenderer() override;

	virtual void render() override;
	virtual void waitIdle() override;

	virtual void registerShader(const std::string& vertPath, const std::string& fragPath) override;
	virtual void removeShader(const std::string& vertPath, const std::string& fragPath) override;

	virtual void registerVertexData(const std::vector<Vertex>& vertexData) override;
	virtual void removeVertexData(const std::vector<Vertex>& vertexData) override;

	void recreateSwapChainAndDependent();

private:
	void initVulkan();
	void cleanupVulkan();

	void drawFrame();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	void createSceneRenderCommands(VkCommandBuffer commandBuffer);

	void createSwapChainAndDependent();
	void cleanSwapChainAndDependent();
};

