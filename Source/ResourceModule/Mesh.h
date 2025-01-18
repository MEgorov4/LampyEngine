#pragma once
#include <glm/glm.hpp>
#include <vector>

struct MeshVertex
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
};

class Mesh 
{
	std::vector<MeshVertex> m_vertexData;
	uint32_t m_uniqueID;
public:
	Mesh(const std::vector<MeshVertex>& vertexData);

	const std::vector<MeshVertex>& getVertexData();
};