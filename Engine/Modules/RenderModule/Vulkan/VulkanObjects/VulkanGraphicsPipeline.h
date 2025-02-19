#pragma once 
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <string>

#include "../../../ResourceModule/Mesh.h"


/// <summary>
/// Manages a Vulkan graphics pipeline, including shaders, input layouts, and rasterization settings.
/// </summary>
class VulkanGraphicsPipeline
{
    VkPipeline m_vk_pipeline; ///< Handle to the Vulkan graphics pipeline.
    VkPipelineLayout m_vk_pipelineLayout; ///< Handle to the Vulkan pipeline layout.
    VkDevice m_vk_device; ///< Handle to the Vulkan logical device.
    uint32_t uniqueID; ///< Unique identifier for this pipeline.

public:
    /// <summary>
    /// Constructs a Vulkan graphics pipeline.
    /// </summary>
    /// <param name="device">The Vulkan logical device.</param>
    /// <param name="renderPass">The Vulkan render pass.</param>
    /// <param name="vertPath">Path to the vertex shader file.</param>
    /// <param name="fragPath">Path to the fragment shader file.</param>
    /// <exception cref="std::runtime_error">Thrown if pipeline creation fails.</exception>
    VulkanGraphicsPipeline(VkDevice device, VkRenderPass renderPass, const std::string& vertPath, const std::string& fragPath);

    /// <summary>
    /// Destroys the Vulkan graphics pipeline and associated resources.
    /// </summary>
    ~VulkanGraphicsPipeline();

    /// <summary>
    /// Retrieves the Vulkan pipeline handle.
    /// </summary>
    /// <returns>Handle to the Vulkan pipeline.</returns>
    VkPipeline getPipeline() const { return m_vk_pipeline; }

    /// <summary>
    /// Retrieves the Vulkan pipeline layout handle.
    /// </summary>
    /// <returns>Handle to the Vulkan pipeline layout.</returns>
    VkPipelineLayout getPipelineLayout() const { return m_vk_pipelineLayout; }

    /// <summary>
    /// Retrieves the unique ID assigned to this pipeline.
    /// </summary>
    /// <returns>Unique ID of the pipeline.</returns>
    uint32_t getUniqueID() { return uniqueID; };

private:
    /// <summary>
    /// Creates a Vulkan shader module from a given bytecode.
    /// </summary>
    /// <param name="code">Binary data of the shader.</param>
    /// <returns>Handle to the Vulkan shader module.</returns>
    /// <exception cref="std::runtime_error">Thrown if shader module creation fails.</exception>
    VkShaderModule createShaderModule(const std::vector<char>& code);
};
