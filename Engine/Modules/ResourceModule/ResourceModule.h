#pragma once
#include <memory>
#include <string>

#include "Shader.h"
#include "../RenderModule/RenderModule.h"
#include "ShaderLoader.h"
class ResourceModule
{
public:
	ResourceModule() {}
	~ResourceModule() {}
	static ResourceModule& getInstance()
	{
		static ResourceModule ResourceManager;
		return ResourceManager;
	}

	void startup()
	{
		LOG_INFO("ResourceModule: Startup");
	}

	std::unique_ptr<RShader> createAndRegisterShader(const std::string& vertPath, const std::string& fragPath)
	{
		// std::unique_ptr<RShader> shader = std::make_unique<RShader>(vertPath, fragPath);
		// assert(shader);

		// RenderModule::getInstance().registerShader(vertPath, fragPath);

		// return shader;
		return nullptr;
	}

	void removeShader(std::unique_ptr<RShader> shader)
	{
		// RenderModule::getInstance().removeShader(shader->getVertPath(), shader->getFragPath());
	} 
	
	std::unique_ptr<RMesh> createAndRegisterMesh(const std::vector<MeshVertex>& vertices)
	{

		std::unique_ptr<RMesh> mesh = std::make_unique<RMesh>(vertices);
		assert(mesh);

		std::vector<Vertex> v_vertices(vertices.size());

		std::transform(vertices.begin(), vertices.end(), v_vertices.begin(),
			[](const MeshVertex& meshVertex)
			{
				return Vertex(meshVertex);
			});

		// RenderModule::getInstance().registerVertexData(v_vertices);
		return mesh;
	}
	
	void removeMesh(std::unique_ptr<RMesh> mesh)
	{
		std::vector<MeshVertex> meshVertices = mesh->getVertexData();
		std::vector<Vertex> vertices(meshVertices.size());

		std::transform(meshVertices.begin(), meshVertices.end(), vertices.begin(),
			[](const MeshVertex& meshVertex)
			{
				return Vertex(meshVertex);
			});

		// RenderModule::getInstance().removeVertexData(vertices);
	}
	
	std::vector<char> loadShader(const std::string& path)
	{
		return ShaderLoader::readShaderFile(path);
	}

	void shutDown()
	{
		LOG_INFO("ResourceModule: Shut down");
	}
};