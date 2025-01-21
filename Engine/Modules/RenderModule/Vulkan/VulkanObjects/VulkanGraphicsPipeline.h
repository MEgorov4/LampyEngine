#pragma once 
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <string>

#include "../../../ResourceModule/Mesh.h"

struct Vertex {
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
    
    Vertex() : pos(0.f), uv(0.f), normal(0.f){}

    Vertex(const MeshVertex& meshVertex)
    {
        pos = meshVertex.pos;
        uv = meshVertex.uv;
        normal = meshVertex.normal;
    }
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
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

class VulkanGraphicsPipeline
{
	VkPipeline m_vk_pipeline;
	VkPipelineLayout m_vk_pipelineLayout;

	VkDevice m_vk_device;
    uint32_t uniqueID;
public:
	VulkanGraphicsPipeline(VkDevice device, VkRenderPass renderPass, const std::string& vertPath, const std::string& fragPath);
	~VulkanGraphicsPipeline();
	
	VkPipeline getPipeline() const { return m_vk_pipeline; }
	VkPipelineLayout getPipelineLayout() const { return m_vk_pipelineLayout; }
    
    uint32_t getUniqueID() { return uniqueID; };
private:
	VkShaderModule createShaderModule(const std::vector<char>& code);
};