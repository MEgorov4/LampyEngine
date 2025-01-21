#pragma once
#include "vulkan/vulkan.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include "VulkanGraphicsPipeline.h"
#include "VulkanVertexBuffer.h"

class VulkanVertexBufferCache
{
    std::unordered_map<std::string, std::pair<uint32_t, std::unique_ptr<VulkanVertexBuffer>>> m_vertexBuffers;

    std::string hashVertexData(const std::vector<Vertex>& vertexData) const;

public:
    VulkanVertexBufferCache();
    VulkanVertexBufferCache(const VulkanVertexBufferCache&) = delete;
    ~VulkanVertexBufferCache();
    VulkanVertexBufferCache& operator=(const VulkanVertexBufferCache& rhs) = delete;

    void clearCache();
    VulkanVertexBuffer* getOrCreateVertexBuffer(const std::vector<Vertex>& vertexData,
        VkQueue transferQueue,
        VkCommandPool commandPool,
        VkDevice device,
        VkPhysicalDevice physicalDevice);
    void removeVertexBuffer(const std::vector<Vertex>& vertexData);
};

