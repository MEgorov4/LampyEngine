#include "Mesh.h"
#include "Foundation/Assert/Assert.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Material.h"
#include "ResourceManager.h"

using EngineCore::Foundation::ResourceAllocator;

namespace std
{
template <> struct hash<ResourceModule::MeshVertex>
{
    size_t operator()(const ResourceModule::MeshVertex &vertex) const
    {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec2>()(vertex.uv) << 1)) >> 1) ^
               (hash<glm::vec3>()(vertex.normal) << 1);
    }
};
}; // namespace std

namespace ResourceModule
{
RMesh::RMesh(const std::string &path) : BaseResource(path)
{
    LT_ASSERT_MSG(!path.empty(), "Mesh path cannot be empty");
    
    // Initialize as empty
    m_mesh.vertices.clear();
    m_mesh.indices.clear();
    m_mesh.aabbMin = glm::vec3(0.0f);
    m_mesh.aabbMax = glm::vec3(0.0f);
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        LT_LOGE("RMesh", "Failed to open meshbin: " + path);
        return; // Leave resource empty
    }

    uint32_t vertexCount = 0, indexCount = 0;
    file.read(reinterpret_cast<char *>(&vertexCount), sizeof(vertexCount));
    file.read(reinterpret_cast<char *>(&indexCount), sizeof(indexCount));

    // Check file read success and validate counts BEFORE allocating memory
    // These checks MUST work in release builds to prevent memory corruption
    if (file.fail())
    {
        LT_LOGE("RMesh", "Failed to read meshbin header: " + path);
        return; // Leave resource empty
    }
    
    // Validate counts to prevent huge memory allocation from corrupted files
    // CRITICAL: These checks prevent 32GB memory leaks - must work in release!
    if (vertexCount == 0)
    {
        LT_LOGE("RMesh", "Mesh has no vertices: " + path);
        return; // Leave resource empty
    }
    
    if (vertexCount >= 1000000)
    {
        LT_LOGE("RMesh", std::string("Mesh vertex count is unreasonably large (") + std::to_string(vertexCount) + "): " + path);
        return; // Leave resource empty - prevent memory leak
    }
    
    if (indexCount >= 10000000)
    {
        LT_LOGE("RMesh", std::string("Mesh index count is unreasonably large (") + std::to_string(indexCount) + "): " + path);
        return; // Leave resource empty - prevent memory leak
    }
    
    // Now safe to allocate memory
    std::vector<float, ResourceAllocator<float>> vertices(vertexCount * 3);
    std::vector<float, ResourceAllocator<float>> normals(vertexCount * 3);
    std::vector<float, ResourceAllocator<float>> texcoords(vertexCount * 2);
    std::vector<uint32_t, ResourceAllocator<uint32_t>> indices(indexCount);

    file.read(reinterpret_cast<char *>(vertices.data()), vertices.size() * sizeof(float));
    file.read(reinterpret_cast<char *>(normals.data()), normals.size() * sizeof(float));
    file.read(reinterpret_cast<char *>(texcoords.data()), texcoords.size() * sizeof(float));
    file.read(reinterpret_cast<char *>(indices.data()), indices.size() * sizeof(uint32_t));

    if (file.fail())
    {
        LT_LOGE("RMesh", "Corrupted meshbin: " + path);
        return; // Leave resource empty - vectors will be destroyed automatically
    }

    m_mesh.vertices.reserve(vertexCount);
    m_mesh.indices.reserve(indexCount);

    m_mesh.aabbMin = glm::vec3(std::numeric_limits<float>::max());
    m_mesh.aabbMax = glm::vec3(std::numeric_limits<float>::lowest());

    for (uint32_t i = 0; i < vertexCount; ++i)
    {
        // Bounds checking - if corrupted, leave resource empty
        if (i * 3 + 2 >= vertices.size() || i * 2 + 1 >= texcoords.size())
        {
            LT_LOGE("RMesh", "Vertex index out of bounds: " + path);
            m_mesh.vertices.clear();
            m_mesh.indices.clear();
            return; // Leave resource empty
        }
        
        MeshVertex v{};
        v.pos = glm::vec3(vertices[i * 3 + 0], vertices[i * 3 + 1], vertices[i * 3 + 2]);
        v.normal = glm::vec3(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]);
        v.uv = glm::vec2(texcoords[i * 2 + 0], texcoords[i * 2 + 1]);

        m_mesh.aabbMin = glm::min(m_mesh.aabbMin, v.pos);
        m_mesh.aabbMax = glm::max(m_mesh.aabbMax, v.pos);
        m_mesh.vertices.push_back(v);
    }

    m_mesh.indices = std::move(indices);
    
    // Final validation
    if (m_mesh.vertices.empty() || m_mesh.indices.empty())
    {
        LT_LOGE("RMesh", "Mesh has no vertices or indices after loading: " + path);
        m_mesh.vertices.clear();
        m_mesh.indices.clear();
        return; // Leave resource empty
    }

    LT_LOGI("RMesh", std::format("Loaded meshbin {} ({} vertices, {} indices)", path, vertexCount, indexCount));
}
} // namespace ResourceModule
