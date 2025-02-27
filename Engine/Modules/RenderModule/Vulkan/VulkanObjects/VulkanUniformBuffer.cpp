#include "VulkanUniformBuffer.h"

#include "../../RenderConfig.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // нужно настроить матрицу перспективной проекции на диапазон от 0.0 до 1.0
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

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

void VulkanUniformBuffer::updateUniformBuffer(uint32_t currentImage, VkExtent2D extent)
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	// существующее преобразование, угол поворота и ось вращения
	for (auto& object : ubo.objects)
	{
		object.model = glm::rotate(glm::mat4(1.f), glm::radians(10.f * glm::sin(time)), glm::vec3(0.f, 0.f, 1.f));
	}
	// ubo.objects[0] = glm::rotate(glm::mat4(1.f), glm::radians(10.f * glm::sin(time)), glm::vec3(0.f, 0.f, 1.f));

	// позиция камеры, центральная точка и ось "вверх"
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.2f, 1.0f), glm::vec3(0.0f, 0.0f, 0.15f), glm::vec3(0.0f, 0.0f, 1.0f));

	// перспективная проекция с углом 45 градусов, соотношение сторон, ближняя и дальняя плоскости отсечения
	ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 100.0f);

	// Так как у Vulkan Y инвертирован, надо его перевернуть, иначе картинка будет перевёрнутой
	ubo.proj[1][1] *= -1;

	// Копируем данные из объекта после преобразования в uniform buffer
	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
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
