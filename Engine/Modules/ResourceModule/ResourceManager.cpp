#include "ResourceManager.h"

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

#include "../../Modules/LoggerModule/Logger.h"

#include "../MemoryModule/DoubleStackAllocator.h"
#include "../ObjectCoreModule/ECS/ECSModule.h"

ResourceManager::ResourceManager()
{
	size_t meshCacheSize = sizeof(RMesh);
	size_t meshNum = 100;

	size_t shaderCacheSize = sizeof(RShader);
	size_t shaderNum = 100;

	size_t textureCacheSize = sizeof(RTexture);
	size_t textureNum = 100;

	size_t sumResources = meshCacheSize * meshNum + shaderCacheSize * shaderNum + textureCacheSize * textureNum;
	size_t sumReverse = 1024 * 1024 * 5;
	m_doubleStackAllocator = new DoubleStackAllocator(sumResources + sumReverse);

	if (m_doubleStackAllocator)
	{
		void* meshCacheAddress = m_doubleStackAllocator->allocateStart(meshCacheSize * meshNum);
		meshCache = MeshCache(meshCacheSize, meshNum, meshCacheAddress);

		void* shaderCacheAddress = m_doubleStackAllocator->allocateStart(shaderCacheSize * shaderNum);
		shaderCache = ShaderCache(shaderCacheSize, shaderNum, shaderCacheAddress);

		void* textureCacheAddress = m_doubleStackAllocator->allocateStart(textureCacheSize * textureNum);
		textureCache = TextureCache(textureCacheSize, textureNum, textureCacheAddress);
	}
}

void ResourceManager::startup()
{
	
}

void ResourceManager::shutDown()
{
	clearAllCache();

	delete m_doubleStackAllocator;

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

void ResourceManager::clearAllCache()
{
	meshCache.clear();
	shaderCache.clear();
	textureCache.clear();
}

void ResourceManager::OnLoadInitialWorldState()
{
	checkAllResources();
}

DoubleStackAllocator* ResourceManager::getDoubleStackAllocator() const
{
	return m_doubleStackAllocator;
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