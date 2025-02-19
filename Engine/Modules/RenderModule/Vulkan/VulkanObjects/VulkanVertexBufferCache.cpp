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

std::string VulkanVertexBufferCache::hashVertexData(const std::vector<Vertex>& vertexData) const
{
    std::ostringstream oss;
    for (const auto& vertex : vertexData)
    {
        oss << std::fixed << std::setprecision(6) << vertex.pos.x
            << vertex.pos.y << vertex.pos.z
            << vertex.normal.r << vertex.normal.g << vertex.normal.b
            << vertex.uv.x << vertex.uv.y;
    }
    return std::to_string(std::hash<std::string>{}(oss.str()));
}

VulkanVertexBuffer* VulkanVertexBufferCache::getOrCreateVertexBuffer(const std::vector<Vertex>& vertexData,
    VkQueue transferQueue,
    VkCommandPool commandPool,
    VkDevice device,
    VkPhysicalDevice physicalDevice)
{
    std::string key = hashVertexData(vertexData);

    auto it = m_vertexBuffers.find(key);
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

    m_vertexBuffers[key] = std::make_pair(1, std::move(newVertexBuffer));
    return m_vertexBuffers[key].second.get();
}

void VulkanVertexBufferCache::removeVertexBuffer(const std::vector<Vertex>& vertexData)
{
    std::string key = hashVertexData(vertexData);

    auto it = m_vertexBuffers.find(key);
    if (it != m_vertexBuffers.end())
    {
        it->second.first--;

        if (it->second.first == 0)
        {
            m_vertexBuffers.erase(it);
        }
    }
}
