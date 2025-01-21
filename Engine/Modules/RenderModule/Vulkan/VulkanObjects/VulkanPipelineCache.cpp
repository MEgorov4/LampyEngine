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
    if (!m_pipelines.empty())
    {
        m_pipelines.clear();
    }
}

VulkanGraphicsPipeline* VulkanPipelineCache::getOrCreatePipeline(const std::string& fragPath, const std::string& vertPath, VkDevice device, VkRenderPass renderPass)
{
    std::string key = vertPath + '|' + fragPath;

    auto it = m_pipelines.find(key);
    if (it != m_pipelines.end())
    {
        it->second.first++; 
        return it->second.second.get();
    }

    std::unique_ptr<VulkanGraphicsPipeline> newPipeline = std::make_unique<VulkanGraphicsPipeline>(device, renderPass, fragPath, vertPath);
    m_pipelines[key] = std::make_pair(1, std::move(newPipeline));

    return m_pipelines[key].second.get();
}

void VulkanPipelineCache::removePipeline(const std::string& fragPath, const std::string& vertPath)
{
    std::string key = vertPath + '|' + fragPath;

    auto it = m_pipelines.find(key);
    if (it != m_pipelines.end())
    {
        it->second.first--;

        if (it->second.first == 0)
        {
            m_pipelines.erase(it);
        }
    }
}

