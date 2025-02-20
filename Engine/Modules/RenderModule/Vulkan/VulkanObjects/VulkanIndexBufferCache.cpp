#include "VulkanIndexBufferCache.h"
#include "VulkanIndexBuffer.h"
#include <sstream>
#include <iomanip>
#include <functional>

VulkanIndexBufferCache::VulkanIndexBufferCache()
{
   
}

VulkanIndexBufferCache::~VulkanIndexBufferCache()
{
    clearCache();
}

void VulkanIndexBufferCache::clearCache()
{
    for (auto& buffer : m_indexBuffers)
    {
        buffer.second.second.get()->cleanupVulkanBuffer();
    }
    m_indexBuffers.clear();
}

VulkanIndexBuffer* VulkanIndexBufferCache::getOrCreateIndexBuffer(const std::vector<uint32_t>& indexData,
    const std::string& pathToFile,
    VkQueue transferQueue,
    VkCommandPool commandPool,
    VkDevice device,
    VkPhysicalDevice physicalDevice)
{
    auto it = m_indexBuffers.find(pathToFile);
    if (it != m_indexBuffers.end())
    {
        it->second.first++;
        return it->second.second.get();
    }

    auto newVertexBuffer = std::make_unique<VulkanIndexBuffer>(device,
        physicalDevice,
        indexData,
        transferQueue,
        commandPool);

    m_indexBuffers[pathToFile] = std::make_pair(1, std::move(newVertexBuffer));
    return m_indexBuffers[pathToFile].second.get();
}

void VulkanIndexBufferCache::removeIndexBuffer(const std::vector<uint32_t>& indexData, const std::string& pathToFile)
{
    auto it = m_indexBuffers.find(pathToFile);
    if (it != m_indexBuffers.end())
    {
        it->second.first--;

        if (it->second.first == 0)
        {
            m_indexBuffers.erase(it);
        }
    }
}