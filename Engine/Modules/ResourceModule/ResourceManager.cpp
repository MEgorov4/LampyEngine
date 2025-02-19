#include "ResourceManager.h"

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

#include "../../Modules/LoggerModule/Logger.h"

#include "../RenderModule/RenderModule.h"
#include "../ObjectCoreModule/ECS/ECSModule.h"
// #include "../RenderModule/Vulkan/VulkanRenderer.h"
// #include "../RenderModule/Vulkan/VulkanObjects/VulkanVertexBufferCache.h"

void ResourceManager::clearAllCache()
{
	meshCache.clear();
	shaderCache.clear();
	textureCache.clear();
}

void ResourceManager::startup()
{
	
}

void ResourceManager::shutDown()
{
	clearAllCache();

	auto& world = ECSModule::getInstance().getCurrentWorld();

	auto query = world.query<MeshComponent, Position>();

	query.each([&](flecs::entity e, MeshComponent& mesh, Position& pos)
		{
			std::shared_ptr<RMesh> loadedMesh = load<RMesh>(std::string(mesh.meshResourcePath));

			if (!loadedMesh) return;

			std::vector<Vertex> vertices(loadedMesh->getVertexData().begin(), loadedMesh->getVertexData().end());
			RenderModule::getInstance().removeVertexData(vertices);
			RenderModule::getInstance().removeIndexData(loadedMesh->getIndicesData());
		});
}

void ResourceManager::OnLoadInitialWorldState()
{
	// LOG_INFO("On Load Initial World State");
	checkAllResources();
}

ResourceManager::ResourceManager()
{
	meshCache = MeshCache();
	shaderCache = ShaderCache();
	textureCache = TextureCache();
}

void ResourceManager::checkAllResources()
{
	auto& world = ECSModule::getInstance().getCurrentWorld();

	auto query = world.query<Position, MeshComponent>();
	query.each([](const flecs::entity& entity, Position& pos, MeshComponent& mesh)
		{
			std::shared_ptr<RMesh> loadedMesh = load<RMesh>(std::string(mesh.meshResourcePath));

			if (!loadedMesh) return;
			
			std::vector<Vertex> vertices(loadedMesh->getVertexData().begin(), loadedMesh->getVertexData().end());
			RenderModule::getInstance().registerVertexData(vertices);
			RenderModule::getInstance().registerIndexData(loadedMesh->getIndicesData());
		});

	flecs::entity bob = world.entity("Bob");
	if (!bob.has<Position>()) 
	{
		std::cout << "Bob does not have Position component!" << std::endl;
	}
	if (!bob.has<MeshComponent>()) 
	{
		std::cout << "Bob does not have MeshComponent component!" << std::endl;
	}
}
