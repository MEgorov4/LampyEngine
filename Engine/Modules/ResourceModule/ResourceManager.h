#pragma once

#include <string>
#include <memory>
#include "ResourceCache.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

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

	void clearAllCache();

	void startup();
	void shutDown();
	void OnLoadInitialWorldState();
private:
	ResourceManager();

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

	return getInstance().meshCache.load(path);
}

template<>
inline std::shared_ptr<RShader> ResourceManager::load<RShader>(const std::string& path)
{
	if (path.size() == 0)
	{
		return nullptr;
	}

	return getInstance().shaderCache.load(path);
}

template<>
inline std::shared_ptr<RTexture> ResourceManager::load<RTexture>(const std::string& path)
{
	if (path.size() == 0)
	{
		return nullptr;
	}

	return getInstance().textureCache.load(path);
}