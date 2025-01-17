#pragma once
#include "vulkan/vulkan.h"
#include "../../VulkanApplication.h"
#include "../../VulkanObjects/VulkanCommandPool.h"
#include "../../VulkanObjects/VulkanLogicalDevice.h"
#include "../../Logger/Logger.h"
#include "Shader.h"
#include "ShaderManager.h"
#include <format>

#include "Mesh.h"
class ResourceManager
{
	VulkanApplication* m_application;

private:
	ResourceManager() {};

public:
	ResourceManager(const ResourceManager& resourceManager) = delete;
	const ResourceManager& operator=(const ResourceManager& rm) = delete;

	static ResourceManager& getResourceManager()
	{
		static ResourceManager resourceManager;
		return resourceManager;
	}
	
	void setApplication(VulkanApplication* application)
	{
		m_application = application;
	}

	std::unique_ptr<Mesh> createMesh(const std::vector<Vertex>& vertices)
	{
		LOG_INFO("ResourceManager::createMesh: Start create mesh");

		checkApplication();

		std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(m_application->getLogicalDevice()->getLogicalDevice(), 
							  m_application->getLogicalDevice()->getPhysicalDevice(),
							  vertices, 
							  m_application->getLogicalDevice()->getGraphicsQueue(),
							  m_application->getCommandPool()->getCommandPool());
		assert(mesh);
		return mesh;
	}
	std::shared_ptr<Shader> createShader(const std::string& vertPath = "vert.spv", const std::string& fragPath = "frag.spv")
	{ 
		LOG_INFO("ResourceManager::createManager: Start create shader" + std::format("\nVertex shader path: {}\nFragment shader path:{}", vertPath, fragPath));

		checkApplication();
	

		std::shared_ptr<Shader> shader = std::make_shared<Shader>(vertPath, fragPath);

		SM_ADD_SHADER(shader, m_application->getLogicalDevice()->getLogicalDevice(), m_application->getRenderPass()->getRenderPass());
		assert(shader);

		return shader;
	}

private:
	void checkApplication()
	{
		if (!m_application)
		{
			throw std::runtime_error("[Error] ResourceManager: application is not valid");
		}
	}
};

#define RM_SET_APPLICATION(application) ResourceManager::getResourceManager().setApplication(application)
#define RM_CREATE_MESH(vertices) ResourceManager::getResourceManager().createMesh(vertices)
#define RM_CREATE_SHADER(vertPath, fragPath) ResourceManager::getResourceManager().createShader(vertPath, fragPath)
