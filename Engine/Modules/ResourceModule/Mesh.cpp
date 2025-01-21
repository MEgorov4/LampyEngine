#include "Mesh.h"

Mesh::Mesh(const std::vector<MeshVertex>& vertexData) : m_vertexData(vertexData)
{
	static uint32_t ID = 0;
	m_uniqueID = ID++;
}

const std::vector<MeshVertex>& Mesh::getVertexData()
{
	return m_vertexData;
}

