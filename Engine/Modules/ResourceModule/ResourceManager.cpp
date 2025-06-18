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
	size_t sumReverse = size_t(1024 * 1024 * 5);
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
}

void ResourceManager::clearAllCache()
{
	meshCache.clear();
	shaderCache.clear();
	textureCache.clear();
}

DoubleStackAllocator* ResourceManager::getDoubleStackAllocator() const
{
	return m_doubleStackAllocator;
}
