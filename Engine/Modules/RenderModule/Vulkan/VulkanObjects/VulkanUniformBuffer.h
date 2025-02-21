#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
#include "VulkanBuffer.h"

const int MAX_OBJECTS = 200;

struct ObjectData 
{
    glm::mat4 model;
};

struct UniformBufferObject
{
    glm::mat4 view;
    glm::mat4 proj;
    ObjectData object[MAX_OBJECTS];
};

class VulkanUniformBuffer : public IVulkanBuffer
{
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkDevice m_device;
public:
    VulkanUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice);

    ~VulkanUniformBuffer();

    void cleanupVulkanUniformBuffers();
private:
    void createUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice);

};