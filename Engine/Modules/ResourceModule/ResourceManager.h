#pragma once

#include <string>
#include <memory>
#include "ResourceCache.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "../FilesystemModule/FilesystemModule.h"
#include "../MemoryModule/DoubleStackAllocator.h"

class DoubleStackAllocator;

using MeshCache = ResourceCache<RMesh>;
using ShaderCache = ResourceCache<RShader>;
using TextureCache = ResourceCache<RTexture>;

class ResourceManager
{
public:
	static ResourceManager& getInstance()
	{
		static ResourceManager resourceManager;
		return resourceManager;
	}

	template<class T>
	static std::shared_ptr<T> load(const std::string& path);
	template<class T>
	static void unload(const std::string& path);

	void clearAllCache();

	void startup();
	void shutDown();
	void OnLoadInitialWorldState();

	DoubleStackAllocator* getDoubleStackAllocator() const;
private:
	ResourceManager();

	DoubleStackAllocator* m_doubleStackAllocator;

	MeshCache meshCache;
	ShaderCache shaderCache;
	TextureCache textureCache;

	void checkAllResources();
};

template<>
inline std::shared_ptr<RMesh> ResourceManager::load<RMesh>(const std::string& path)
{
	if (path.size() == 0)
	{
		return nullptr;
	}

	return getInstance().meshCache.load(FS.getEngineAbsolutePath(path));
}

template<>
inline std::shared_ptr<RShader> ResourceManager::load<RShader>(const std::string& path)
{
	if (path.size() == 0)
	{
		return nullptr;
	}

	return getInstance().shaderCache.load(FS.getEngineAbsolutePath(path));
}

template<>
inline std::shared_ptr<RTexture> ResourceManager::load<RTexture>(const std::string& path)
{
	if (path.size() == 0)
	{
		return nullptr;
	}

	return getInstance().textureCache.load(FS.getEngineAbsolutePath(path));
}

template<>
inline void ResourceManager::unload<RMesh>(const std::string& path)
{
	getInstance().meshCache.unload(FS.getEngineAbsolutePath(path));
}

template<>
inline void ResourceManager::unload<RShader>(const std::string& path)
{
	getInstance().shaderCache.unload(FS.getEngineAbsolutePath(path));
}

template<>
inline void ResourceManager::unload<RTexture>(const std::string& path)
{
	getInstance().textureCache.unload(FS.getEngineAbsolutePath(path));
}