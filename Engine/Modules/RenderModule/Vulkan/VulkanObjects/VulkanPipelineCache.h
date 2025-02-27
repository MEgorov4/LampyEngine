#pragma once
#include "vulkan/vulkan.h"
#include <unordered_map>
#include <string>
#include <memory>

class VulkanGraphicsPipeline;

/// <summary>
/// Manages a cache of Vulkan graphics pipelines to optimize resource reuse and reduce redundant pipeline creation.
/// </summary>
class VulkanPipelineCache
{
    /// <summary>
    /// Stores cached graphics pipelines mapped by a key (combining vertex and fragment shader paths).
    /// Each entry consists of a reference count and a unique pointer to a VulkanGraphicsPipeline.
    /// </summary>
    std::unordered_map<std::string, std::pair<uint32_t, std::unique_ptr<VulkanGraphicsPipeline>>> m_pipelines;

public:
    /// <summary>
    /// Constructs an empty Vulkan pipeline cache.
    /// </summary>
    VulkanPipelineCache();

    /// <summary>
    /// Deleted copy constructor to prevent copying.
    /// </summary>
    VulkanPipelineCache(const VulkanPipelineCache&) = delete;

    /// <summary>
    /// Destroys all cached pipelines.
    /// </summary>
    ~VulkanPipelineCache();

    /// <summary>
    /// Deleted assignment operator to prevent copying.
    /// </summary>
    /// <param name="rhs">The right-hand side object to assign from.</param>
    /// <returns>Reference to this object.</returns>
    VulkanPipelineCache& operator=(const VulkanPipelineCache&) = delete;

    /// <summary>
    /// Clears the pipeline cache, removing all stored pipelines.
    /// </summary>
    void clearCache();

    /// <summary>
    /// Retrieves an existing Vulkan graphics pipeline or creates a new one if it does not exist.
    /// </summary>
    /// <param name="fragPath">Path to the fragment shader file.</param>
    /// <param name="vertPath">Path to the vertex shader file.</param>
    /// <param name="device">The Vulkan logical device.</param>
    /// <param name="renderPass">The Vulkan render pass.</param>
    /// <returns>Pointer to the Vulkan graphics pipeline.</returns>
    VulkanGraphicsPipeline* getOrCreatePipeline(const std::string& fragPath, const std::string& vertPath, VkDevice device, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);

    /// <summary>
    /// Removes a Vulkan graphics pipeline from the cache if it is no longer referenced.
    /// </summary>
    /// <param name="fragPath">Path to the fragment shader file.</param>
    /// <param name="vertPath">Path to the vertex shader file.</param>
    void removePipeline(const std::string& fragPath, const std::string& vertPath);
};
