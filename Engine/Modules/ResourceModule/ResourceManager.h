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
	static ResourceManager& instance()
	{
		static ResourceManager resourceManager;
		return resourceManager;
	}

	template<class T>
	static std::shared_ptr<T> load(const std::string& path);

	void clearAllCache();
private:
	ResourceManager();

	MeshCache meshCache;
	ShaderCache shaderCache;
	TextureCache textureCache;
};

template<>
inline std::shared_ptr<RMesh> ResourceManager::load<RMesh>(const std::string& path)
{
	if (path.size() == 0)
	{
		return nullptr;
	}

	return instance().meshCache.load(path);
}

template<>
inline std::shared_ptr<RShader> ResourceManager::load<RShader>(const std::string& path)
{
	if (path.size() == 0)
	{
		return nullptr;
	}

	return instance().shaderCache.load(path);
}

template<>
inline std::shared_ptr<RTexture> ResourceManager::load<RTexture>(const std::string& path)
{
	if (path.size() == 0)
	{
		return nullptr;
	}

	return instance().textureCache.load(path);
}