#pragma once

#include <EngineMinimal.h>

#include "ResourceCache.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

namespace ShaderCompiler
{
	class ShaderCompiler;
}

namespace ResourceModule
{
	class RMesh;
	class RShader;
	class RTexture;

	class ResourceManager : public IModule
	{
		ShaderCompiler::ShaderCompiler* m_shaderCompiler;

		ResourceCache<RMesh> meshCache;
		ResourceCache<RShader> shaderCache;
		ResourceCache<RTexture> textureCache;
	public:
		void startup() override;
		void shutdown() override;

		template<class T>
		std::shared_ptr<T> load(const std::string& path);
		
		template<class T>
		void unload(const std::string& path);

		void clearAllCache();
	};
	
	template <>
	inline std::shared_ptr<RMesh> ResourceManager::load<RMesh>(const std::string& path)
	{
        LT_LOGI("ResourceManager", "Try load mesh: " + path);
		if (path.empty())
		{
			return nullptr;
		}
		return meshCache.load(Fs::absolutePath(path));
	}

	template <>
	inline std::shared_ptr<RShader> ResourceManager::load<RShader>(const std::string& path)
	{
        LT_LOGI("ResourceManager", "Try load shader: " + path);
		if (path.empty())
		{
			return nullptr;
		}

		return shaderCache.load(Fs::absolutePath(path));
	}

	template <>
	inline std::shared_ptr<RTexture> ResourceManager::load<RTexture>(const std::string& path)
	{
        LT_LOGI("ResourceManager", "Try load texture: " + path);
		if (path.empty())
		{
			return nullptr;
		}

		return textureCache.load(Fs::absolutePath(path));
	}

	template <>
	inline void ResourceManager::unload<RMesh>(const std::string& path)
	{
        LT_LOGI("ResourceManager", "Try unload mesh: " + path);
		meshCache.unload<RMesh>(Fs::absolutePath(path));
	}

	template <>
	inline void ResourceManager::unload<RShader>(const std::string& path)
	{
        LT_LOGI("ResourceManager", "Try unload shader: " + path);
		shaderCache.unload<RShader>(Fs::absolutePath(path));
	}

	template <>
	inline void ResourceManager::unload<RTexture>(const std::string& path)
	{
        LT_LOGI("ResourceManager", "Try unload texture: " + path);
		textureCache.unload<RTexture>(Fs::absolutePath(path));
	}
}
