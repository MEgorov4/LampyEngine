#include "Mesh.h"
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <stdexcept>
#include <glm/gtx/hash.hpp>

#include "Material.h"
#include "ResourceManager.h"

namespace std
{
	template<> struct hash<MeshVertex>
	{
		size_t operator()(MeshVertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec2>()(vertex.uv) << 1)) >> 1) ^
				(hash<glm::vec3>()(vertex.normal) << 1);
		}
	};
};

RMesh::RMesh(const std::string& path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
	{
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<MeshVertex, uint32_t> uniqueVertices{};

	for (const tinyobj::shape_t& shape : shapes)
	{
		for (const tinyobj::index_t& index : shape.mesh.indices)
		{
			MeshVertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0], 
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.uv = {
				attrib.texcoords[2 * index.texcoord_index + 0], 
				1.f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			// vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(m_vertexData.size());
				m_vertexData.push_back(vertex);
			}

			m_indicesData.push_back(uniqueVertices[vertex]);
		}
	}

	m_materials.resize(materials.size());
	/*for (const tinyobj::material_t& material : materials)
	{
		std::shared_ptr<RMaterial> mat = ResourceManager::load<RMaterial>(std::string(""));
		m_materials.push_back(mat.get());
	}*/

	vertPath = "D:/B_Projects/LampyEngine/Resources/Shaders/GLSL/vert.spv";
	fragPath = "D:/B_Projects/LampyEngine/Resources/Shaders/GLSL/frag.spv";
}

const std::vector<MeshVertex>& RMesh::getVertexData()
{
	return m_vertexData;
}

const std::vector<uint32_t>& RMesh::getIndicesData()
{
	return m_indicesData;
}