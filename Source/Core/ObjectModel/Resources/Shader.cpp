#include "Shader.h"
#include "ShaderManager.h"

Shader::Shader(const std::string& vertPath, const std::string& fragPath) : m_vertPath(vertPath), m_fragPath(fragPath)
{
}

void Shader::recreatePipeline(VkDevice device, VkRenderPass renderPass)
{
}

void Shader::bindShader(VkCommandBuffer commandBuffer)
{
	VulkanGraphicsPipeline* pipeline =  SM_GET_PIPELINE_BY_HASH(getShaderHash());
	if (pipeline)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());
	}
}

std::string Shader::getShaderHash()
{
	return m_vertPath + '|' + m_fragPath;
}

std::string Shader::getVertPath()
{
	return m_vertPath;
}

std::string Shader::getFragPath()
{
	return m_fragPath;
}
