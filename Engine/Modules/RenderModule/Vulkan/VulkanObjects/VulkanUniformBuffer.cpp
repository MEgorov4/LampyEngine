#include "VulkanUniformBuffer.h"

#include "../../RenderConfig.h"

VulkanUniformBuffer::VulkanUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice) : m_device(device)
{
	createUniformBuffer(device, physicalDevice);
}

VulkanUniformBuffer::~VulkanUniformBuffer()
{

}

void VulkanUniformBuffer::cleanupVulkanUniformBuffers()
{
	const uint32_t maxFramesInFlight = RenderConfig::getInstance().getMaxFramesInFlight();
	for (size_t i = 0; i < maxFramesInFlight; ++i)
	{
		if (uniformBuffersMapped[i]) 
		{
			vkUnmapMemory(m_device, uniformBuffersMemory[i]);
			uniformBuffersMapped[i] = nullptr;
		}

		vkDestroyBuffer(m_device, uniformBuffers[i], nullptr);
		vkFreeMemory(m_device, uniformBuffersMemory[i], nullptr);
	}
}

void VulkanUniformBuffer::createUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice)
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	const uint32_t maxFramesInFlight = RenderConfig::getInstance().getMaxFramesInFlight();

	uniformBuffers.resize(maxFramesInFlight);
	uniformBuffersMemory.resize(maxFramesInFlight);
	uniformBuffersMapped.resize(maxFramesInFlight);

	for (size_t i = 0; i < maxFramesInFlight; ++i)
	{
		createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

		vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}
}
