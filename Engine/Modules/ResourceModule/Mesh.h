#pragma once
#include "BaseResource.h"
#include <EngineMinimal.h>

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
    std::vector<MeshVertex, ProfileAllocator<MeshVertex>> vertices;
    std::vector<uint32_t, ProfileAllocator<uint32_t>> indices;
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

  private:
    MeshData m_mesh;
};
} // namespace ResourceModule
