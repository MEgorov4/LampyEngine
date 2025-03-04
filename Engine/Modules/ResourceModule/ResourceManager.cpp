#include "ResourceManager.h"

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

#include "../../Modules/LoggerModule/Logger.h"

#include "../ObjectCoreModule/ECS/ECSModule.h"

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

	//auto& world = ECSModule::getInstance().getCurrentWorld();

	//auto query = world.query<MeshComponent, PositionComponent>();

	//query.each([&](flecs::entity e, MeshComponent& mesh, PositionComponent& pos)
	//	{
	//		const std::string meshPath = std::string(mesh.meshResourcePath);
	//		std::shared_ptr<RMesh> loadedMesh = load<RMesh>(meshPath);

	//		if (!loadedMesh) return;

	//		std::vector<Vertex> vertices(loadedMesh->getVertexData().begin(), loadedMesh->getVertexData().end());
	//		RenderModule::getInstance().removeVertexData(vertices, meshPath);
	//		RenderModule::getInstance().removeIndexData(loadedMesh->getIndicesData(), meshPath);
	//	});
}

void ResourceManager::OnLoadInitialWorldState()
{
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
	//auto& world = ECSModule::getInstance().getCurrentWorld();

	//auto query = world.query<PositionComponent, MeshComponent>();
	//query.each([](const flecs::entity& entity, PositionComponent& pos, MeshComponent& mesh)
	//	{
	//		const std::string meshPath = std::string(mesh.meshResourcePath);
	//		std::shared_ptr<RMesh> loadedMesh = load<RMesh>(meshPath);

	//		if (!loadedMesh) return;
	//		
	//		std::vector<Vertex> vertices(loadedMesh->getVertexData().begin(), loadedMesh->getVertexData().end());
	//		RenderModule::getInstance().registerVertexData(vertices, meshPath);
	//		RenderModule::getInstance().registerIndexData(loadedMesh->getIndicesData(), meshPath);
	//	});
}
