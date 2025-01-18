#pragma once
#include <format>

#include "Shader.h"
#include "Mesh.h"
#include "../../../LoggerModule/Logger.h"
class ResourceManager
{
private:
	ResourceManager() {};

public:
	ResourceManager(const ResourceManager& resourceManager) = delete;
	const ResourceManager& operator=(const ResourceManager& rhs) = delete;

	static ResourceManager& getResourceManager()
	{
		static ResourceManager resourceManager;
		return resourceManager;
	}

	std::unique_ptr<Mesh> createMesh(const std::vector<MeshVertex>& vertices)
	{
		LOG_INFO("ResourceManager::createMesh: Start create mesh");

		std::unique_ptr<Mesh> mesh;
		return mesh;
	}
	std::shared_ptr<Shader> createShader(const std::string& vertPath = "vert.spv", const std::string& fragPath = "frag.spv")
	{ 
		LOG_INFO("ResourceManager::createManager: Start create shader" + std::format("\nVertex shader path: {}\nFragment shader path:{}", vertPath, fragPath));

		std::shared_ptr<Shader> shader = std::make_shared<Shader>(vertPath, fragPath);

		assert(shader);

		return shader;
	}

};

#define RM_CREATE_MESH(vertices) ResourceManager::getResourceManager().createMesh(vertices)
#define RM_CREATE_SHADER(vertPath, fragPath) ResourceManager::getResourceManager().createShader(vertPath, fragPath)
