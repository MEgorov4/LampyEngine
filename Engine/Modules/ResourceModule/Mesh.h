#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "BaseResource.h"
#include "../MemoryModule/GarbageAllocator.h"

class RMaterial;

struct MeshVertex
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;

	bool operator==(const MeshVertex& other) const
	{
		return pos == other.pos && uv == other.uv && normal == other.normal;
	}
};

using GarbageString = std::basic_string<char, std::char_traits<char>, GarbageAllocator<char>>;

class RMesh : public BaseResource
{
public:
	RMesh(const std::string& path);

	const std::vector<MeshVertex> getVertexData();
	const std::vector<uint32_t> getIndicesData();

	const glm::vec3& getAABBCenter() const;
	const glm::vec3& getAABBSize() const;
private:
	std::vector<MeshVertex, GarbageAllocator<MeshVertex>> m_vertexData;
	std::vector<uint32_t, GarbageAllocator<uint32_t>> m_indicesData;

	std::vector<RMaterial*, GarbageAllocator<RMaterial*>> m_materials;

	glm::vec3 aabbMin;
	glm::vec3 aabbMax;
public:
	GarbageString vertPath;
	GarbageString fragPath;
};