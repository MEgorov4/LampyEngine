#pragma once
#include "BaseResource.h"
#include <EngineMinimal.h>
#include "Foundation/Memory/ResourceAllocator.h"

using EngineCore::Foundation::ResourceAllocator;

namespace ResourceModule
{
struct MeshVertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct MeshData
{
    std::vector<MeshVertex, ResourceAllocator<MeshVertex>> vertices;
    std::vector<uint32_t, ResourceAllocator<uint32_t>> indices;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
};

class RMesh : public BaseResource
{
  public:
    explicit RMesh(const std::string &path);
    ~RMesh() noexcept = default;

    const MeshData &getMeshData() const noexcept
    {
        return m_mesh;
    }

    bool isValid() const noexcept
    {
        return !m_mesh.vertices.empty() && !m_mesh.indices.empty();
    }

    bool isEmpty() const noexcept
    {
        return m_mesh.vertices.empty() && m_mesh.indices.empty();
    }

  private:
    MeshData m_mesh;
};
} // namespace ResourceModule
