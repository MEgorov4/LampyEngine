#pragma once
#include "../../VulkanObjects/VulkanGraphicsPipeline.h"
#include "../../VulkanObjects/VulkanVertexBuffer.h"
#include <memory>
class Mesh 
{
	static uint32_t ID;

	uint32_t m_uniqueID;
	std::unique_ptr<VulkanVertexBuffer> m_vertexBuffer;
public:
	Mesh(VkDevice device,
		VkPhysicalDevice physicalDevice,
		const std::vector<Vertex>& vertexData,
		VkQueue transferQueue,
		VkCommandPool commandPool);

	void renderMesh(VkCommandBuffer commandBuffer, glm::mat4 modelMatrix);
};