#include "ShaderManager.h"
#include "../../../LoggerModule/Logger.h"
#include "format"
#include "../../VulkanObjects/VulkanGraphicsPipeline.h"
ShaderManager& ShaderManager::getShaderManager()
{
	static ShaderManager shaderManager;
	return shaderManager;
}

void ShaderManager::addShader(std::shared_ptr<Shader> shader, VkDevice device, VkRenderPass renderPass)
{
	std::string key = shader->getShaderHash();
	auto it = m_pipelines.find(key);
	if (it == m_pipelines.end())
	{
		m_pipelines[key] = std::make_unique<VulkanGraphicsPipeline>(device, renderPass, shader->getVertPath(), shader->getFragPath());
	}
	m_shaders.push_back(shader);
}

void ShaderManager::recreateShaders(VkDevice device, VkRenderPass renderPass)
{
	for (auto shader : m_shaders)
	{
		std::string key = shader->getShaderHash();
		m_pipelines[key].reset();
		m_pipelines[key] = std::make_unique<VulkanGraphicsPipeline>(device, renderPass, shader->getVertPath(), shader->getFragPath());

		shader->recreatePipeline(device, renderPass);
	}
}

void ShaderManager::resetPipeline()
{
	m_current_pipeline = nullptr;
}

VulkanGraphicsPipeline* ShaderManager::getPipelineByHash(const std::string& hash)
{
	VulkanGraphicsPipeline* pipeline = m_pipelines[hash].get();
	if (pipeline == m_current_pipeline)
	{
		return nullptr;
	}
	m_current_pipeline = pipeline;
	return m_pipelines[hash].get();
}
