#include "VulkanPipelineCache.h"

#include "VulkanGraphicsPipeline.h"

VulkanPipelineCache::VulkanPipelineCache()
{
}

VulkanPipelineCache::~VulkanPipelineCache()
{
	clearCache();
}

void VulkanPipelineCache::clearCache()
{
	m_pipelines.clear();
}

VulkanGraphicsPipeline* VulkanPipelineCache::getOrCreatePipeline(const std::string fragPath, const std::string vertPath, VkDevice device, VkRenderPass renderPass)
{
	std::string key = vertPath + '|' + fragPath;
	auto it = m_pipelines.find(key);
	if (it != m_pipelines.end())
	{
		return it->second.get();
	}

	m_pipelines[key] = std::make_unique<VulkanGraphicsPipeline>(device, renderPass, fragPath, vertPath);

	return m_pipelines[key].get();
}
