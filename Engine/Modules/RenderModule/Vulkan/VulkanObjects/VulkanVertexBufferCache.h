#pragma once
#include "vulkan/vulkan.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include "VulkanGraphicsPipeline.h"
#include "VulkanVertexBuffer.h"

/// <summary>
/// Caches and manages Vulkan vertex buffers to prevent redundant buffer creation.
/// </summary>
class VulkanVertexBufferCache
{
    /// <summary>
    /// Stores cached vertex buffers mapped by their hash string.
    /// Each entry consists of a reference count and a unique pointer to a VulkanVertexBuffer.
    /// </summary>
    std::unordered_map<std::string, std::pair<uint32_t, std::unique_ptr<VulkanVertexBuffer>>> m_vertexBuffers;

    /// <summary>
    /// Generates a unique hash string for a given vertex data set.
    /// </summary>
    /// <param name="vertexData">Vector containing vertex data.</param>
    /// <returns>Unique hash string representing the vertex data.</returns>
    std::string hashVertexData(const std::vector<Vertex>& vertexData) const;

public:
    /// <summary>
    /// Constructs an empty Vulkan vertex buffer cache.
    /// </summary>
    VulkanVertexBufferCache();

    /// <summary>
    /// Deleted copy constructor to prevent copying of the cache.
    /// </summary>
    VulkanVertexBufferCache(const VulkanVertexBufferCache&) = delete;

    /// <summary>
    /// Destroys the cache and clears all stored vertex buffers.
    /// </summary>
    ~VulkanVertexBufferCache();

    /// <summary>
    /// Deleted assignment operator to prevent copying.
    /// </summary>
    /// <param name="rhs">The right-hand side object to assign from.</param>
    /// <returns>Reference to this object.</returns>
    VulkanVertexBufferCache& operator=(const VulkanVertexBufferCache& rhs) = delete;

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
    VulkanVertexBuffer* getOrCreateVertexBuffer(const std::vector<Vertex>& vertexData,
        VkQueue transferQueue,
        VkCommandPool commandPool,
        VkDevice device,
        VkPhysicalDevice physicalDevice);

    /// <summary>
    /// Removes a vertex buffer from the cache if it is no longer needed.
    /// </summary>
    /// <param name="vertexData">Vertex data associated with the buffer.</param>
    void removeVertexBuffer(const std::vector<Vertex>& vertexData);
};
