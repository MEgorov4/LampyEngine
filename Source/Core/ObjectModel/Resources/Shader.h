#pragma once
#include "../../VulkanObjects/VulkanGraphicsPipeline.h"
#include <memory>
#include <string>

class Shader
{
	std::string m_vertPath;
	std::string m_fragPath;
public:
	Shader(const std::string& vertPath, const std::string& fragPath);
	
	void recreatePipeline(VkDevice device, VkRenderPass renderPass);

	void bindShader(VkCommandBuffer commandBuffer);

	std::string getShaderHash();
	std::string getVertPath();
	std::string getFragPath();
};