#include "Mesh.h"

uint32_t Mesh::ID = 0;

Mesh::Mesh(VkDevice device, VkPhysicalDevice physicalDevice, const std::vector<Vertex>& vertexData, VkQueue transferQueue, VkCommandPool commandPool)
{
	m_uniqueID = ID++;

	m_vertexBuffer = std::make_unique<VulkanVertexBuffer>(device, physicalDevice, vertexData, transferQueue, commandPool);
}

void Mesh::renderMesh(VkCommandBuffer commandBuffer, glm::mat4 modelMatrix)
{
	VkBuffer vertexBuffers[] = { m_vertexBuffer->getBuffer()};
	VkDeviceSize offsets[] = { 0 };
	
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdDraw(commandBuffer, m_vertexBuffer->getVerticesCount(), 1, m_uniqueID, 0);
}
