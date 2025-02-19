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

std::string VulkanIndexBufferCache::hashIndexData(const std::vector<uint32_t>& indexData) const
{
    std::size_t seed = 0;
    for (uint32_t value : indexData)
    {
        seed ^= std::hash<uint32_t>{}(value)+0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return std::to_string(seed);
}

VulkanIndexBuffer* VulkanIndexBufferCache::getOrCreateIndexBuffer(const std::vector<uint32_t>& indexData,
    VkQueue transferQueue,
    VkCommandPool commandPool,
    VkDevice device,
    VkPhysicalDevice physicalDevice)
{
    std::string key = hashIndexData(indexData);

    auto it = m_indexBuffers.find(key);
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

    m_indexBuffers[key] = std::make_pair(1, std::move(newVertexBuffer));
    return m_indexBuffers[key].second.get();
}

void VulkanIndexBufferCache::removeIndexBuffer(const std::vector<uint32_t>& indexData)
{
    std::string key = hashIndexData(indexData);

    auto it = m_indexBuffers.find(key);
    if (it != m_indexBuffers.end())
    {
        it->second.first--;

        if (it->second.first == 0)
        {
            m_indexBuffers.erase(it);
        }
    }
}