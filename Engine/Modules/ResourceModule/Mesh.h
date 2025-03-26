#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "BaseResource.h"

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


class RMesh : public BaseResource
{
public:
	RMesh(const std::string& path);

	const std::vector<MeshVertex>& getVertexData();
	const std::vector<uint32_t>& getIndicesData();
	
private:
	std::vector<MeshVertex> m_vertexData;
	std::vector<uint32_t> m_indicesData;

	std::vector<RMaterial*> m_materials;

public:
	std::string vertPath;
	std::string fragPath;
};