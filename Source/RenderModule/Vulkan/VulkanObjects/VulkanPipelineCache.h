#pragma once
#include "vulkan/vulkan.h"
#include <unordered_map>
#include <string>
#include <memory>

class VulkanGraphicsPipeline;

class VulkanPipelineCache
{
	std::unordered_map<std::string, std::unique_ptr<VulkanGraphicsPipeline>> m_pipelines;
public:
	VulkanPipelineCache();
	VulkanPipelineCache(const VulkanPipelineCache&) = delete;
	~VulkanPipelineCache();
	VulkanPipelineCache& operator=(const VulkanPipelineCache&) = delete;

	void clearCache();
	VulkanGraphicsPipeline* getOrCreatePipeline(const std::string fragPath, const std::string vertPath, VkDevice device, VkRenderPass renderPass);
};