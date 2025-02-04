#pragma once 
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <string>

#include "../../../ResourceModule/Mesh.h"

/// <summary>
/// Represents a single vertex structure used in Vulkan rendering.
/// </summary>
struct Vertex {
    glm::vec3 pos;    ///< Vertex position (x, y, z).
    glm::vec2 uv;     ///< Texture coordinates (u, v).
    glm::vec3 normal; ///< Normal vector (x, y, z).

    /// <summary>
    /// Default constructor initializes the vertex with zero values.
    /// </summary>
    Vertex() : pos(0.f), uv(0.f), normal(0.f) {}

    /// <summary>
    /// Constructs a Vertex from a MeshVertex structure.
    /// </summary>
    /// <param name="meshVertex">The mesh vertex data to copy from.</param>
    Vertex(const MeshVertex& meshVertex)
    {
        pos = meshVertex.pos;
        uv = meshVertex.uv;
        normal = meshVertex.normal;
    }

    /// <summary>
    /// Returns the Vulkan binding description for a vertex buffer.
    /// </summary>
    /// <returns>VkVertexInputBindingDescription structure.</returns>
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    /// <summary>
    /// Returns the Vulkan attribute descriptions for a vertex buffer.
    /// </summary>
    /// <returns>An array of VkVertexInputAttributeDescription.</returns>
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, uv);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, normal);

        return attributeDescriptions;
    }
};

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
