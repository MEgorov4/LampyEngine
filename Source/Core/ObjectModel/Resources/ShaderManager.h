#pragma once 
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Shader.h"
class ShaderManager
{
	std::unordered_map<std::string, std::unique_ptr<VulkanGraphicsPipeline>> m_pipelines;
	VulkanGraphicsPipeline* m_current_pipeline;
	std::vector<std::shared_ptr<Shader>> m_shaders;

	ShaderManager() { m_current_pipeline = nullptr; };
public:
	static ShaderManager& getShaderManager();
	void addShader(std::shared_ptr<Shader> shader, VkDevice device, VkRenderPass renderPass);

	void recreateShaders(VkDevice device, VkRenderPass renderPass);
	
	VulkanGraphicsPipeline* getPipelineByHash(const std::string& hash);

	void resetPipeline();
};

#define SM_ADD_SHADER(shader, device, renderPass) ShaderManager::getShaderManager().addShader(shader, device, renderPass)
#define SM_RECREATE_SHADERS(device, renderPass) ShaderManager::getShaderManager().recreateShaders(device, renderPass)
#define SM_GET_PIPELINE_BY_HASH(hash) ShaderManager::getShaderManager().getPipelineByHash(hash)
#define SM_RESET_PIPELINE() ShaderManager::getShaderManager().resetPipeline()