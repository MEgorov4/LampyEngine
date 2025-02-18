#include "VulkanRenderer.h"
#include <stdexcept>

#include <imgui.h>

#include "../../WindowModule/Window.h"

#include "VulkanObjects/VulkanInstance.h"
#include "VulkanObjects/VulkanSurface.h"
#include "VulkanObjects/VulkanLogicalDevice.h"
#include "VulkanObjects/VulkanSwapChain.h"
#include "VulkanObjects/VulkanRenderPass.h"
#include "VulkanObjects/VulkanFramebuffers.h"
#include "VulkanObjects/VulkanCommandPool.h"
#include "VulkanObjects/VulkanCommandBuffers.h"
#include "VulkanObjects/VulkanSynchronizationManager.h"

#include "VulkanObjects/VulkanVertexBufferCache.h"
#include "VulkanObjects/VulkanPipelineCache.h"
#include "VulkanObjects/VulkanDescriptorPool.h"
#include "VulkanOffscreenObjects/VulkanOffscreenRenderer.h"
#include "../RenderConfig.h"
#include "../../WindowModule/WindowModule.h"
#include "../../ImGuiModule/GLFWBackends/imgui_impl_glfw.h"
#include "../../ImGuiModule/VulkanBackends/imgui_impl_vulkan.h"
#include "../../ObjectCoreModule/ECS/ECSModule.h"
VulkanRenderer::VulkanRenderer(Window* window) : m_window(window)
{
	initVulkan();
	m_imguiEnabled = RenderConfig::getInstance().getImGuiEnabled();
	if (m_imguiEnabled)
	{
		initImGui();
	}
}
VulkanRenderer::~VulkanRenderer() 
{
	m_mainDeletionQueue.flush();
	cleanupVulkan();
}

void VulkanRenderer::render()
{
	drawFrame();
}

void VulkanRenderer::initVulkan()
{
	m_instance = std::make_unique<VulkanInstance>(m_window->getRequiredInstanceExtensions(),
												  true);
	m_surface = std::make_unique<VulkanSurface>(m_instance->getInstance(), 
												m_window->getWindowSurface(m_instance->getInstance()));
	m_logicalDevice = std::make_unique<VulkanLogicalDevice>(m_instance->getInstance(),
															m_surface->getSurface());

	m_commandPool = std::make_unique<VulkanCommandPool>(m_logicalDevice->getLogicalDevice(),
														m_logicalDevice->getDeviceFamilyIndices().graphicsFamily.value());
	createSwapChainAndDependent();
	m_pipelineCache = std::make_unique<VulkanPipelineCache>();
	m_vertexBufferCache = std::make_unique<VulkanVertexBufferCache>();

	m_syncManager = std::make_unique<VulkanSynchronizationManager>(m_logicalDevice->getLogicalDevice(),
														   RenderConfig::getInstance().getMaxFramesInFlight());

	m_descriptorPool = std::make_unique<VulkanDescriptorPool>(m_logicalDevice->getLogicalDevice());

	m_offscreenRenderer = std::make_unique<VulkanOffscreenRenderer>(m_logicalDevice->getLogicalDevice()
		, m_logicalDevice->getPhysicalDevice()
		, VkExtent2D(1920, 1080)
		, m_commandPool->getCommandPool()
		, m_logicalDevice->getGraphicsQueue()
		, m_descriptorPool->getDescriptorPool());
}

void VulkanRenderer::initImGui() {
	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
	poolInfo.pPoolSizes = pool_sizes;

	VkDescriptorPool imguiPool;
	if (vkCreateDescriptorPool(m_logicalDevice->getLogicalDevice(), &poolInfo, nullptr, &imguiPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create ImGui descriptor pool!");
	}

	ImGui::CreateContext();
	assert(ImGui::GetCurrentContext() != nullptr && "Failed to create ImGui context!");

	ImGui_ImplGlfw_InitForVulkan(WindowModule::getInstance().getWindow()->getGLFWWindow(), true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_instance->getInstance();
	init_info.PhysicalDevice = m_logicalDevice->getPhysicalDevice();
	init_info.Device = m_logicalDevice->getLogicalDevice();
	init_info.Queue = m_logicalDevice->getGraphicsQueue();
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = RenderConfig::getInstance().getMaxFramesInFlight();
	init_info.ImageCount = RenderConfig::getInstance().getMaxFramesInFlight();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, m_renderPass->getRenderPass());

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->AddFontFromFileTTF("../Fonts/JetBrainsMono-Light.ttf", 16);
	io.Fonts->AddFontFromFileTTF("../Fonts/JetBrainsMono-Regular.ttf", 16);
	io.Fonts->AddFontFromFileTTF("../Fonts/JetBrainsMono-Light.ttf", 32);
	io.Fonts->AddFontFromFileTTF("../Fonts/JetBrainsMono-Regular.ttf", 11);
	io.Fonts->AddFontFromFileTTF("../Fonts/JetBrainsMono-Bold.ttf", 11);
	io.Fonts->Build();

	immediate_submit([&](VkCommandBuffer cmd) {
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
		});

	ImGui_ImplVulkan_DestroyFontUploadObjects();

	m_mainDeletionQueue.push_function([=]() {
		vkDestroyDescriptorPool(m_logicalDevice->getLogicalDevice(), imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
		});
	
}


void VulkanRenderer::immediate_submit(std::function<void(VkCommandBuffer)>&& function) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool->getCommandPool(); 
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer cmd;
	vkAllocateCommandBuffers(m_logicalDevice->getLogicalDevice(), &allocInfo, &cmd);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd, &beginInfo);

	function(cmd);

	vkEndCommandBuffer(cmd);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	vkQueueSubmit(m_logicalDevice->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_logicalDevice->getGraphicsQueue());

	vkFreeCommandBuffers(m_logicalDevice->getLogicalDevice(), m_commandPool->getCommandPool(), 1, &cmd);
}

void VulkanRenderer::cleanupVulkan()
{
	m_offscreenRenderer.reset();
	m_descriptorPool.reset();
	m_syncManager.reset();
	cleanSwapChainAndDependent();
	m_commandPool.reset();
	m_logicalDevice.reset();
	m_surface.reset();
	m_instance.reset();
}

void VulkanRenderer::recreateSwapChainAndDependent()
{
	m_logicalDevice->deviceWaitIdle();

	cleanSwapChainAndDependent();
	createSwapChainAndDependent();
}


void VulkanRenderer::cleanSwapChainAndDependent()
{
	m_commandBuffers.reset();     
	m_framebuffers.reset();       
	m_renderPass.reset();         
	m_pipelineCache->clearCache();
	m_swapChain.reset();          
}

void VulkanRenderer::createSwapChainAndDependent()
{
	m_swapChain = std::make_unique<VulkanSwapChain>(m_logicalDevice->getLogicalDevice(),
		m_surface->getSurface(),
		m_window->getExtent(),
		m_logicalDevice->getDeviceSwapChainSupportDetails(),
		m_logicalDevice->getDeviceFamilyIndices());

	m_renderPass = std::make_unique<VulkanRenderPass>(m_logicalDevice->getLogicalDevice(),
		m_swapChain->getSurfaceFormat().format);

	m_framebuffers = std::make_unique<VulkanFramebuffers>(m_logicalDevice->getLogicalDevice(),
		m_renderPass->getRenderPass(),
		m_swapChain->getExtent(),
		m_swapChain->getImageViews());

	m_commandBuffers = std::make_unique<VulkanCommandBuffers>(
		m_logicalDevice->getLogicalDevice(),
		m_commandPool->getCommandPool(),
		RenderConfig::getInstance().getMaxFramesInFlight()
	);
}

void VulkanRenderer::waitIdle()
{
	m_logicalDevice->deviceWaitIdle();
}

void VulkanRenderer::registerShader(const std::string& vertPath, const std::string& fragPath)
{
	m_pipelineCache->getOrCreatePipeline(vertPath, 
										 fragPath, 
										 m_logicalDevice->getLogicalDevice(), 
										 m_renderPass->getRenderPass());
}

void VulkanRenderer::removeShader(const std::string& vertPath, const std::string& fragPath)
{
	m_pipelineCache->removePipeline(vertPath, fragPath);
}

void VulkanRenderer::registerVertexData(const std::vector<Vertex>& vertexData)
{
	m_vertexBufferCache->getOrCreateVertexBuffer(vertexData,
												 m_logicalDevice->getGraphicsQueue(),
												 m_commandPool->getCommandPool(),
												 m_logicalDevice->getLogicalDevice(),
												 m_logicalDevice->getPhysicalDevice());
}

void VulkanRenderer::removeVertexData(const std::vector<Vertex>& vertexData)
{
	m_vertexBufferCache->removeVertexBuffer(vertexData);
}

void* VulkanRenderer::getVulkanOffscreenImageView()
{
	return m_offscreenRenderer->getColorImageDescriptor();
}

void VulkanRenderer::drawFrame()
{
	VkExtent2D actualExtent = m_window->getExtent();
	if (actualExtent.width <= 0 || actualExtent.height <= 0)
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
		recreateSwapChainAndDependent();
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

	if ((oldExtent.width != actualExtent.width) || (oldExtent.height != actualExtent.width))
	{
		if ((result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->wasResized())) {
			m_window->resetResizedFlag();
			recreateSwapChainAndDependent();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

	m_currentFrame = (m_currentFrame + 1) % RenderConfig::getInstance().getMaxFramesInFlight();
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
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

	if (m_imguiEnabled)
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	}

	vkCmdEndRenderPass(commandBuffer);

	m_offscreenRenderer->beginRenderPass(commandBuffer);
	
	createSceneRenderCommands(commandBuffer);

	m_offscreenRenderer->endRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void VulkanRenderer::createSceneRenderCommands(VkCommandBuffer commandBuffer)
{
	if (m_rendererScene)
	{
		/*
		auto& world = ECSModule::getInstance().getCurrentWorld();

		auto query = world.query<Mesh, Position, Rotation, Scale>();

		query.each()*/
	}
}

