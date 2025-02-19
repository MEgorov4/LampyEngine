#pragma once

#include "vulkan/vulkan.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include "VulkanGraphicsPipeline.h"
#include "VulkanIndexBuffer.h"

/// <summary>
/// Caches and manages Vulkan Index buffers to prevent redundant buffer creation.
/// </summary>
class VulkanIndexBufferCache
{
    /// <summary>
    /// Stores cached vertex buffers mapped by their hash string.
    /// Each entry consists of a reference count and a unique pointer to a VulkanVertexBuffer.
    /// </summary>
    std::unordered_map<std::string, std::pair<uint32_t, std::unique_ptr<VulkanIndexBuffer>>> m_indexBuffers;

    /// <summary>
    /// Generates a unique hash string for a given vertex data set.
    /// </summary>
    /// <param name="indexData">Vector containing index data.</param>
    /// <returns>Unique hash string representing the vertex data.</returns>
    std::string hashIndexData(const std::vector<uint32_t>& indexData) const;

public:
    VulkanIndexBufferCache();
    VulkanIndexBufferCache(const VulkanIndexBufferCache&) = delete;
    ~VulkanIndexBufferCache();
    VulkanIndexBufferCache& operator=(const VulkanIndexBufferCache& rhs) = delete;

    /// <summary>
    /// Clears the cache by removing all stored vertex buffers.
    /// </summary>
    void clearCache();

    /// <summary>
    /// Retrieves an existing vertex buffer or creates a new one if it doesn't exist.
    /// </summary>
    /// <param name="vertexData">Vertex data for the buffer.</param>
    /// <param name="transferQueue">Vulkan queue for buffer transfers.</param>
    /// <param name="commandPool">Command pool for buffer operations.</param>
    /// <param name="device">Vulkan logical device.</param>
    /// <param name="physicalDevice">Vulkan physical device.</param>
    /// <returns>Pointer to the Vulkan vertex buffer.</returns>
    VulkanIndexBuffer* getOrCreateIndexBuffer(const std::vector<uint32_t>& indexData,
        VkQueue transferQueue,
        VkCommandPool commandPool,
        VkDevice device,
        VkPhysicalDevice physicalDevice);

    /// <summary>
    /// Removes a vertex buffer from the cache if it is no longer needed.
    /// </summary>
    /// <param name="vertexData">Vertex data associated with the buffer.</param>
    void removeIndexBuffer(const std::vector<uint32_t>& indexData);
};