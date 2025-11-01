#pragma once
#include <EngineMinimal.h>
#include "BaseResource.h"

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
        std::vector<MeshVertex> vertices;
        std::vector<uint32_t> indices;
        glm::vec3 aabbMin;
        glm::vec3 aabbMax;
    };

    class RMesh : public BaseResource
    {
    public:
        explicit RMesh(const std::string& path);
        ~RMesh() noexcept = default;

        const MeshData& getMeshData() const noexcept { return m_mesh; }

    private:
        MeshData m_mesh;
    };
}
