#include "VulkanOffscreenRenderer.h"

#include "../../../../Modules/LoggerModule/Logger.h"

#include "../VulkanObjects/Vertex.h"
#include "../../../ObjectCoreModule/ECS/ECSModule.h"
#include "../VulkanObjects/VulkanPipelineCache.h"

#include "../../../ResourceModule/ResourceManager.h"
#include "../../../ResourceModule/Mesh.h"

#include "../../RenderConfig.h"

#include "../VulkanUtils.h"

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

VulkanOffscreenRenderer::~VulkanOffscreenRenderer()
{
	// vkFreeDescriptorSets(device, descriptorPool, 1, &descriptorSet);

	vkDestroySampler(device, colorSampler, nullptr);
	vkDestroyImageView(device, textureImageView, nullptr);
	vkDestroyImage(device, textureImage, nullptr);
	vkFreeMemory(device, textureImageMemory, nullptr);

	// vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroyImageView(device, colorImageView, nullptr);
	vkDestroyImage(device, colorImage, nullptr);
	vkFreeMemory(device, colorImageMemory, nullptr);

	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	for (auto framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
	// vkDestroyFramebuffer(device, framebuffer, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	m_uniformBuffer->cleanupVulkanUniformBuffers();
	m_vertexBufferCache->clearCache();
	m_indexBufferCache->clearCache();
}

void VulkanOffscreenRenderer::beginRenderPass(VkCommandBuffer cmd, uint32_t imageIndex)
{
	VkRenderPassBeginInfo rpBeginInfo{};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = renderPass;
	rpBeginInfo.framebuffer = framebuffers[imageIndex];
	rpBeginInfo.renderArea.offset = { 0, 0 };
	rpBeginInfo.renderArea.extent = extent;

	// ����� ���������� ��� �������� �������: ��� ����� � �������.
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { { 0.f, 0.f, 0.f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	rpBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	rpBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(cmd, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// ������������� viewport � scissor ������� �������� ��������-�����������.
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void VulkanOffscreenRenderer::endRenderPass(VkCommandBuffer cmd)
{
	vkCmdEndRenderPass(cmd);
}

void VulkanOffscreenRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VulkanPipelineCache* vulkanCache, uint32_t currentFrame)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	beginRenderPass(commandBuffer, currentFrame);

	recordWorldRenderCommands(commandBuffer, vulkanCache, currentFrame);

	endRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}

void VulkanOffscreenRenderer::recordWorldRenderCommands(VkCommandBuffer commandBuffer, VulkanPipelineCache* vulkanCache, uint32_t currentFrame)
{
	auto& world = ECSModule::getInstance().getCurrentWorld();

	auto query = world.query<PositionComponent, MeshComponent>();

	query.each([&](const flecs::entity& e, PositionComponent& pos, MeshComponent& mesh)
		{
			const std::string meshPath = std::string(mesh.meshResourcePath);

			auto& resourceManager = ResourceManager::getInstance();
			std::shared_ptr<RMesh> loadedMesh = resourceManager.load<RMesh>(std::string(meshPath));

			if (!loadedMesh)
			{
				LOG_INFO("Can't load mesh in path: " + std::string(meshPath));
			}

			std::vector<Vertex> vertices(loadedMesh->getVertexData().begin(), loadedMesh->getVertexData().end());
			VulkanVertexBuffer* vertexBuffer = m_vertexBufferCache->getOrCreateVertexBuffer(vertices,
				meshPath,
				this->graphicsQueue,
				this->commandPool,
				this->device,
				this->physicalDevice);

			VkBuffer verBuffer = vertexBuffer->getBuffer();

			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &verBuffer, offsets);

			VulkanIndexBuffer* indexBuffer = m_indexBufferCache->getOrCreateIndexBuffer(loadedMesh->getIndicesData(),
				meshPath,
				this->graphicsQueue,
				this->commandPool,
				this->device,
				this->physicalDevice);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

			VulkanGraphicsPipeline* graphicsPipeline = vulkanCache->getOrCreatePipeline(loadedMesh->vertPath, loadedMesh->fragPath, device, renderPass, descriptorSetLayout);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getPipeline());
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getPipelineLayout(), 0, 1, &descriptorSets[currentFrame], 0, nullptr);
			vkCmdDrawIndexed(commandBuffer, indexBuffer->getIndexCount(), 1, 0, 0, 0);
		});
}

void VulkanOffscreenRenderer::submitOffscreenRender(VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore)
{
	// ������ fence, ����� ���������, ��� ��������� ��������
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VkFence renderFence;
	if (vkCreateFence(device, &fenceInfo, nullptr, &renderFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create fence for offscreen rendering!");
	}

	// ������������� ������������� ����������
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// ������� ������� ����� ������� ����������
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &waitSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;

	// ���������� ��������� ���� (��������, ������������� ��� ImGui), ��� ��������� ��������
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &signalSemaphore;

	// ���������� ������� �� GPU
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, renderFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit offscreen command buffer!");
	}

	// ��� ���������� ����������
	if (vkWaitForFences(device, 1, &renderFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
		throw std::runtime_error("Failed to wait for offscreen render fence!");
	}

	// ������� fence
	vkDestroyFence(device, renderFence, nullptr);
}

void VulkanOffscreenRenderer::registerVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile)
{
	m_vertexBufferCache->getOrCreateVertexBuffer(vertexData,
		pathToFile,
		this->graphicsQueue,
		this->commandPool,
		this->device,
		this->physicalDevice);
}

void VulkanOffscreenRenderer::removeVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile)
{
	m_vertexBufferCache->removeVertexBuffer(vertexData, pathToFile);
}

void VulkanOffscreenRenderer::registerIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile)
{
	m_indexBufferCache->getOrCreateIndexBuffer(indexData,
		pathToFile,
		this->graphicsQueue,
		this->commandPool,
		this->device,
		this->physicalDevice);
}

void VulkanOffscreenRenderer::removeIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile)
{
	m_indexBufferCache->removeIndexBuffer(indexData, pathToFile);
}

void VulkanOffscreenRenderer::createTextureImage()
{
	//int texWidth, texHeight, texChannels;
	//stbi_uc* pixels = stbi_load("D:/B_Projects/LampyEngine/Resources/Textures/viking_room.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	//// ������� ������������� �� ������� � 4 ������� �� �������, ������� ��������
	//VkDeviceSize imageSize = texWidth * texHeight * 4;

	//// ��������� ���������� ������� � mip chain. max - �������� ���������� ���������, log2 - ���������, 
	//// ������� ��� ��� ����� ��������� �� 2, 
	//// floor - ������������ ������, ����� ���������� ��������� �� �������� �������� 2, 
	//// 1 - �����������, ����� �������� ����������� ����� ������� MIP.
	//mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	//if (!pixels)
	//{
	//	throw std::runtime_error("Failed to load texture image!");
	//}

	//VkBuffer stagingBuffer;
	//VkDeviceMemory stagingBufferMemory;

	//Utils::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice, surface);
	//void* data;
	//vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	//memcpy(data, pixels, static_cast<size_t>(imageSize));
	//vkUnmapMemory(device, stagingBufferMemory);

	//stbi_image_free(pixels);

	//// VK_IMAGE_USAGE_TRANSFER_DST_BIT - ���������, ��� �������� ���� ������ � VK_IMAGE_USAGE_SAMPLED_BIT - ��� �� ����� ����� ������ � ����������� �� �������.
	//// VK_IMAGE_USAGE_TRANSFER_SRC_BIT - ���������, ��� ����� ������������ ���������� ��� ��������
	//Utils::createImage(device, physicalDevice, texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	//	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	//// ������ ��� ����������� ����� � �����������, ��� ���������� ��������� ����������� � ���������� �����. ��� ����� ���������� ��������� ��������� ��������� ������
	//// ���������� ���� ������� � ������� VK_IMAGE_LAYOUT_UNDEFINED, ������ ��� VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL - ����������� ������������� ��� ������� ������ ��� ����������� ��� ��������
	//Utils::transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, commandPool, device, graphicsQueue);
	//Utils::copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), commandPool, device, graphicsQueue);
	//// ��������� ����������� � ����� VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL - ����������� ������������ ������ ��� ������ � �������� � �� ����� ���� ��������������
	//Utils::generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, commandPool, device, graphicsQueue, physicalDevice);

	//// Utils::transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, commandPool, device, graphicsQueue);

	//vkDestroyBuffer(device, stagingBuffer, nullptr);
	//vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanOffscreenRenderer::createTextureImageView()
{
	textureImageView = Utils::createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, device);
}

void VulkanOffscreenRenderer::updateUniform(uint32_t currentImage)
{
	m_uniformBuffer->updateUniformBuffer(currentImage, extent);
}

void VulkanOffscreenRenderer::init()
{
	m_vertexBufferCache = std::make_unique<VulkanVertexBufferCache>();
	m_indexBufferCache = std::make_unique<VulkanIndexBufferCache>();
	m_uniformBuffer = std::make_unique<VulkanUniformBuffer>(device, physicalDevice);

	createColorResources();
	createDepthResources();
	createRenderPass();
	createFramebuffer();
	createTextureImage();
	createTextureImageView();
	createSampler();
	createDescriptorSetLayout();
	allocateOffscreenDescriptorSet();
	// updateOffscreenDescriptorSet();
}

void VulkanOffscreenRenderer::createColorResources()
{
	// ����� ������ ������, ���������� ��� ����������� � ImGui.
	VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
	createImage(colorFormat,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		colorImage,
		colorImageMemory);
	colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanOffscreenRenderer::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();
	createImage(depthFormat,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		depthImage,
		depthImageMemory);
	// ��� �������� ������������� ���������, ��� ��� ���������� ������ �������.
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanOffscreenRenderer::createRenderPass()
{
	// �������� �������� ��������� ������������ (��� ����� �����������, ������� ������������ � �������� ���������� ��� �������� �������� ������)
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = msaaSamples;

	// ��������� � �������� � ��������� ������
	// �������, ��� ����� ����� ���������� �������� �� ������� ����� ���������� ������ �����
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// ��������� ���������� ����� ���������� � ������, ����� ��� ����� ���� ��������� �����
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	// ��������� ������������ ������
	// ��� �� �������, � ����� ������ ����������� ���������� �����
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// �� �� �����, ����� �������������������� ����������� ���� ������������ �������� (���� ��� � �� ����� ���� ������������)
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = Utils::findDepthFormat(physicalDevice);
	depthAttachment.samples = msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// ��������� ������������ ��� ����� � ����� ������� - ����������� �������������:
	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	// �������� ������ � ������� ������������
	colorAttachmentRef.attachment = 0;
	// ����� �������������� ��� �������� �����
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// RenderPass'� ����� ������� � ���, ��� ���������� resolve (���������) ������������������� ����������� � �������
	// ������ ������ �� �����������, ������� ����� ������� ����� ����������
	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = nullptr/*&colorAttachmentResolveRef*/;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment/*, colorAttachmentResolve*/ };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // �������, ����� �������� �� ��� 
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass!");
	}
}

void VulkanOffscreenRenderer::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	// ���������, �� ����� ���� ���������� ���������� ���������
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	// ������� � ��������� �����������
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

void VulkanOffscreenRenderer::allocateOffscreenDescriptorSet()
{
	const uint32_t maxFrames = RenderConfig::getInstance().getMaxFramesInFlight();

	std::vector<VkDescriptorSetLayout> layouts(maxFrames, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(maxFrames);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(maxFrames);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < maxFrames; ++i)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffer->getBufferByIndex(i);
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = colorSampler;

		// ������������ ������������ ����������� � ������� vkUpdateDescriptorSets �������, ������� ��������� ������ VkWriteDescriptorSet�������� � �������� ���������
		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		// �������, ��� ����������� ��� ������
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// ����������� ���������� ��������� ���
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr; // Optional
		descriptorWrites[0].pTexelBufferView = nullptr; // Optional

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// ����������� ���������� ��������� ���
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = nullptr; // Optional
		descriptorWrites[1].pImageInfo = &imageInfo;
		descriptorWrites[1].pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void VulkanOffscreenRenderer::updateOffscreenDescriptorSet()
{
	//VkDescriptorImageInfo imageInfo{};
	//imageInfo.sampler = colorSampler;
	//imageInfo.imageView = colorImageView;
	//imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	//VkWriteDescriptorSet writeDescSet{};
	//writeDescSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//writeDescSet.dstSet = descriptorSet;
	//writeDescSet.dstBinding = 0; // Binding, ������� �� ������������ � ������������� ������ ��� �������
	//writeDescSet.dstArrayElement = 0;
	//writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//writeDescSet.descriptorCount = 1;
	//writeDescSet.pImageInfo = &imageInfo;

	//vkUpdateDescriptorSets(device, 1, &writeDescSet, 0, nullptr);
}

void VulkanOffscreenRenderer::createFramebuffer()
{
	framebuffers.resize(m_imageViews.size());

	for (size_t i = 0; i < m_imageViews.size(); ++i)
	{
		std::array<VkImageView, 2> attachments = { colorImageView, depthImageView };

		VkFramebufferCreateInfo fbInfo{};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.renderPass = renderPass;
		fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbInfo.pAttachments = attachments.data();
		fbInfo.width = extent.width;
		fbInfo.height = extent.height;
		fbInfo.layers = 1;

		if (vkCreateFramebuffer(device, &fbInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create offscreen framebuffer!");
		}
	}
}

void VulkanOffscreenRenderer::createSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &colorSampler) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create offscreen texture sampler!");
	}
}

void VulkanOffscreenRenderer::createImage(VkFormat format, VkImageUsageFlags usage, VkImage& image, VkDeviceMemory& memory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = format;
	imageInfo.extent.width = extent.width;
	imageInfo.extent.height = extent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = usage;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create offscreen image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate offscreen image memory!");
	}

	vkBindImageMemory(device, image, memory, 0);
}

VkImageView VulkanOffscreenRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create offscreen image view!");
	}
	return imageView;
}
