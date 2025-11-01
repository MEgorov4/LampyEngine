#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Material.h"
#include "ResourceManager.h"

namespace std
{
template <> struct hash<ResourceModule::MeshVertex>
{
    size_t operator()(const ResourceModule::MeshVertex& vertex) const
    {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec2>()(vertex.uv) << 1)) >> 1) ^
               (hash<glm::vec3>()(vertex.normal) << 1);
    }
};
}; // namespace std

namespace ResourceModule
{
RMesh::RMesh(const std::string& path) : BaseResource(path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open meshbin: " + path);

    uint32_t vertexCount = 0, indexCount = 0;
    file.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
    file.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));

    if (vertexCount == 0)
        throw std::runtime_error("Invalid meshbin (no vertices): " + path);

    // --- Чтение данных ---
    std::vector<float> vertices(vertexCount * 3);
    std::vector<float> normals(vertexCount * 3);
    std::vector<float> texcoords(vertexCount * 2);
    std::vector<uint32_t> indices(indexCount);

    file.read(reinterpret_cast<char*>(vertices.data()), vertices.size() * sizeof(float));
    file.read(reinterpret_cast<char*>(normals.data()), normals.size() * sizeof(float));
    file.read(reinterpret_cast<char*>(texcoords.data()), texcoords.size() * sizeof(float));
    file.read(reinterpret_cast<char*>(indices.data()), indices.size() * sizeof(uint32_t));

    if (file.fail())
        throw std::runtime_error("Corrupted meshbin: " + path);

    // --- Заполнение структуры MeshData ---
    m_mesh.vertices.reserve(vertexCount);
    m_mesh.indices.reserve(indexCount);

    m_mesh.aabbMin = glm::vec3(std::numeric_limits<float>::max());
    m_mesh.aabbMax = glm::vec3(std::numeric_limits<float>::lowest());

    for (uint32_t i = 0; i < vertexCount; ++i)
    {
        MeshVertex v{};
        v.pos    = glm::vec3(vertices[i * 3 + 0], vertices[i * 3 + 1], vertices[i * 3 + 2]);
        v.normal = glm::vec3(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]);
        v.uv     = glm::vec2(texcoords[i * 2 + 0], texcoords[i * 2 + 1]);

        m_mesh.aabbMin = glm::min(m_mesh.aabbMin, v.pos);
        m_mesh.aabbMax = glm::max(m_mesh.aabbMax, v.pos);
        m_mesh.vertices.push_back(v);
    }

    m_mesh.indices = std::move(indices);

    LT_LOGI("RMesh", std::format("Loaded meshbin {} ({} vertices, {} indices)", path, vertexCount, indexCount));
}
} // namespace ResourceModule