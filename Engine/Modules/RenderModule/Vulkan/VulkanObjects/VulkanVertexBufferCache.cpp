#include "VulkanVertexBufferCache.h"
#include "VulkanVertexBuffer.h"
#include <sstream>
#include <iomanip>
#include <functional>

VulkanVertexBufferCache::VulkanVertexBufferCache()
{
}

VulkanVertexBufferCache::~VulkanVertexBufferCache()
{
    clearCache();
}

void VulkanVertexBufferCache::clearCache()
{
    for (auto& buffer : m_vertexBuffers)
    {
        buffer.second.second.get()->cleanupVulkanBuffer();
    }
    m_vertexBuffers.clear();
}

VulkanVertexBuffer* VulkanVertexBufferCache::getOrCreateVertexBuffer(const std::vector<Vertex>& vertexData,
    const std::string& pathToFile,
    VkQueue transferQueue,
    VkCommandPool commandPool,
    VkDevice device,
    VkPhysicalDevice physicalDevice)
{
    auto it = m_vertexBuffers.find(pathToFile);
    if (it != m_vertexBuffers.end())
    {
        it->second.first++;
        return it->second.second.get();
    }

    auto newVertexBuffer = std::make_unique<VulkanVertexBuffer>(device, 
                                                                physicalDevice, 
                                                                vertexData,
                                                                transferQueue,
                                                                commandPool);

    m_vertexBuffers[pathToFile] = std::make_pair(1, std::move(newVertexBuffer));
    return m_vertexBuffers[pathToFile].second.get();
}

void VulkanVertexBufferCache::removeVertexBuffer(const std::vector<Vertex>& vertexData, const std::string& pathToFile)
{
    auto it = m_vertexBuffers.find(pathToFile);
    if (it != m_vertexBuffers.end())
    {
        it->second.first--;

        if (it->second.first == 0)
        {
            m_vertexBuffers.erase(it);
        }
    }
}
