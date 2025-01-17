#include "VulkanApplication.h"
#include <stdexcept>
#include "Window/Window.h"

#include "VulkanObjects/VulkanInstance.h"
#include "VulkanObjects/VulkanSurface.h"
#include "VulkanObjects/VulkanLogicalDevice.h"

#include "VulkanObjects/VulkanSwapChain.h"
#include "VulkanObjects/VulkanRenderPass.h"
#include "VulkanObjects/VulkanGraphicsPipeline.h"
#include "VulkanObjects/VulkanFramebuffers.h"

#include "VulkanObjects/VulkanCommandPool.h"
#include "VulkanObjects/VulkanCommandBuffers.h"

#include "VulkanObjects/VulkanVertexBuffer.h"
#include "VulkanObjects/VulkanRenderer.h"
#include "VulkanObjects/VulkanSynchronizationManager.h"

#include "VulkanApplicationConfig.h"

#include <iostream>
#include "Logger/Logger.h"
#include "ObjectModel/Resources/ResourceManager.h"

#include "ObjectModel/Resources/Mesh.h"
#include "Utils/ShaderLoader.h"


VulkanApplication::VulkanApplication()
{
}

VulkanApplication::~VulkanApplication()
{	
}

void VulkanApplication::run()
{
	initWindow();
	initVulkan();
	initEngine();

	mainLoop();
}

void VulkanApplication::initEngine()
{
	RM_SET_APPLICATION(this);
	
	m_scene = new Scene();

	//const std::vector<Vertex> vertices {
	//	{{0.0f, -0.9f}, { 1.0f, 0.0f, 0.0f }},
	//	{ {0.9f, 0.9f}, {0.0f, 1.0f, 0.0f} },
	//	{ {-0.9f, 0.9f}, {0.0f, 0.0f, 1.0f} }};

	//m_mesh = new MeshInstance(nullptr, RM_CREATE_MESH(vertices), RM_CREATE_SHADER("vert.spv", "frag.spv"));
}

void VulkanApplication::initWindow()
{
	m_window = std::make_unique<Window>(800, 600, "Lampy Engine");
}

void VulkanApplication::initVulkan()
{
	m_instance = std::make_unique<VulkanInstance>(m_window->getRequiredInstanceExtensions(), true);
	
	m_surface = std::make_unique<VulkanSurface>(m_instance->getInstance(),
												m_window->getWindowSurface(m_instance->getInstance()));
	m_logicalDevice = std::make_unique<VulkanLogicalDevice>(m_instance->getInstance(), 
															m_surface->getSurface());
	m_swapChain = std::make_unique<VulkanSwapChain>(m_logicalDevice->getLogicalDevice(),
													m_surface->getSurface(),
													m_window->getExtent(),
													m_logicalDevice->getDeviceSwapChainSupportDetails(),
													m_logicalDevice->getDeviceFamilyIndices());
	m_renderPass = std::make_unique<VulkanRenderPass>(m_logicalDevice->getLogicalDevice(), 
													  m_swapChain->getSurfaceFormat().format);
	//m_graphicsPipeline = std::make_unique<VulkanGraphicsPipeline>(m_logicalDevice->getLogicalDevice(),
	//															  m_renderPass->getRenderPass(), "vert.spv", "frag.spv");
	
	m_framebuffers = std::make_unique<VulkanFramebuffers>(m_logicalDevice->getLogicalDevice(), 
														  m_renderPass->getRenderPass(), 
														  m_window->getExtent(),
														  m_swapChain->getImageViews());

	m_commandPool = std::make_unique<VulkanCommandPool>(m_logicalDevice->getLogicalDevice(), 
														m_logicalDevice->getDeviceFamilyIndices().graphicsFamily.value());
	m_commandBuffers = std::make_unique<VulkanCommandBuffers>(
		m_logicalDevice->getLogicalDevice(),
		m_commandPool->getCommandPool(),
		VulkanApplicationConfig::getInstance().getMaxFramesInFlight()
	);

	
	m_syncManager = std::make_unique<VulkanSynchronizationManager>(m_logicalDevice->getLogicalDevice(), VulkanApplicationConfig::getInstance().getMaxFramesInFlight());
}

void VulkanApplication::mainLoop()
{
	while (!m_window->shouldClose())
	{
		m_window->pollEvents();
		drawFrame();
	}

	m_logicalDevice->deviceWaitIdle();
}

void VulkanApplication::drawFrame()
{
	VkExtent2D extent = m_window->getExtent();
	if (extent.width <= 0 || extent.height <= 0)
		return;

	vkWaitForFences(
		m_logicalDevice->getLogicalDevice(),
		1,
		&m_syncManager->getInFlightFence(m_currentFrame),
		VK_TRUE,
		UINT64_MAX
	);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		m_logicalDevice->getLogicalDevice(),
		m_swapChain->getSwapChain(),
		UINT64_MAX,
		m_syncManager->getImageAvailableSemaphore(m_currentFrame),
		VK_NULL_HANDLE,
		&imageIndex
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	vkResetFences(
		m_logicalDevice->getLogicalDevice(),
		1,
		&m_syncManager->getInFlightFence(m_currentFrame)
	);

	vkResetCommandBuffer(
		m_commandBuffers->getCommandBuffers()[m_currentFrame],
		/*VkCommandBufferResetFlagBits*/ 0
	);

	recordCommandBuffer(
		m_commandBuffers->getCommandBuffers()[m_currentFrame],
		imageIndex
	);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_syncManager->getImageAvailableSemaphore(m_currentFrame) };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers->getCommandBuffers()[m_currentFrame];

	VkSemaphore signalSemaphores[] = { m_syncManager->getRenderFinishedSemaphore(m_currentFrame) };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(
		m_logicalDevice->getGraphicsQueue(),
		1,
		&submitInfo,
		m_syncManager->getInFlightFence(m_currentFrame)
	)
		!= VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChain->getSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(m_logicalDevice->getPresentQueue(), &presentInfo);

	VkExtent2D oldExtent = m_swapChain->getExtent();
	VkExtent2D newExtent = m_window->getExtent();

	if((oldExtent.width != newExtent.width) || (newExtent.height != newExtent.width))
	{
		if ((result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->wasResized())) {
			m_window->resetResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

	m_currentFrame = (m_currentFrame + 1) % VulkanApplicationConfig::getInstance().getMaxFramesInFlight();
}

void VulkanApplication::recordCommandBuffer(
	VkCommandBuffer commandBuffer,
	uint32_t imageIndex
) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass->getRenderPass();
	renderPassInfo.framebuffer = m_framebuffers->getFramebuffers()[imageIndex];

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapChain->getExtent();

	VkClearValue clearColor = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapChain->getExtent().width;
	viewport.height = (float)m_swapChain->getExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapChain->getExtent();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	//m_mesh->render(commandBuffer);
	m_scene->RenderScene(commandBuffer);
	SM_RESET_PIPELINE();
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void VulkanApplication::createSwapChainAndDependentResources()
{
	m_swapChain = std::make_unique<VulkanSwapChain>(m_logicalDevice->getLogicalDevice(),
		m_surface->getSurface(),
		m_window->getExtent(),
		m_logicalDevice->getDeviceSwapChainSupportDetails(),
		m_logicalDevice->getDeviceFamilyIndices());

	m_renderPass = std::make_unique<VulkanRenderPass>(m_logicalDevice->getLogicalDevice(),
		m_swapChain->getSurfaceFormat().format);

	SM_RECREATE_SHADERS(m_logicalDevice->getLogicalDevice(), m_renderPass->getRenderPass());

	m_framebuffers = std::make_unique<VulkanFramebuffers>(m_logicalDevice->getLogicalDevice(),
		m_renderPass->getRenderPass(),
		m_swapChain->getExtent(),
		m_swapChain->getImageViews());

	m_commandBuffers = std::make_unique<VulkanCommandBuffers>(
		m_logicalDevice->getLogicalDevice(),
		m_commandPool->getCommandPool(),
		VulkanApplicationConfig::getInstance().getMaxFramesInFlight()
	);
}

void VulkanApplication::cleanupSwapChainAndDependentResources()
{
	m_commandBuffers.reset();     
	m_framebuffers.reset();       
	m_renderPass.reset();         
	m_swapChain.reset();          
}

void VulkanApplication::recreateSwapChain() 
{
	m_logicalDevice->deviceWaitIdle();

	cleanupSwapChainAndDependentResources();

	createSwapChainAndDependentResources();
}
