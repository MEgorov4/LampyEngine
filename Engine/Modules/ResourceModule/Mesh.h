#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

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


class RMesh 
{
public:
	RMesh(const std::string& path);
	RMesh(const std::vector<MeshVertex>& f) {}

	const std::vector<MeshVertex>& getVertexData();

private:
	std::vector<MeshVertex> m_vertexData;
	std::vector<uint32_t> m_indicesData;

	std::vector<RMaterial*> m_materials;
};