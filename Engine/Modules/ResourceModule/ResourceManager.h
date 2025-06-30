#pragma once

#include <string>
#include <memory>

#include "../../EngineContext/IModule.h"
#include "../../EngineContext/ModuleRegistry.h"

#include "../FilesystemModule/FilesystemModule.h"
#include "ResourceCache.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

namespace ShaderCompiler
{
	class ShaderCompiler;
}



namespace Logger
{
	class Logger;
}

namespace ResourceModule
{
	class RMesh;
	class RShader;
	class RTexture;

	class ResourceManager : public IModule
	{
		std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
		std::shared_ptr<ShaderCompiler::ShaderCompiler> m_shaderCompiler;
		std::shared_ptr<Logger::Logger> m_logger;

		ResourceCache<RMesh> meshCache;
		ResourceCache<RShader> shaderCache;
		ResourceCache<RTexture> textureCache;
	public:
		void startup(const ModuleRegistry& registry) override;
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
		if (path.empty())
		{
			return nullptr;
		}

		return meshCache.load(m_filesystemModule->getEngineAbsolutePath(path));
	}

	template <>
	inline std::shared_ptr<RShader> ResourceManager::load<RShader>(const std::string& path)
	{
		if (path.empty())
		{
			return nullptr;
		}

		return shaderCache.load(m_filesystemModule->getEngineAbsolutePath(path), m_filesystemModule, m_shaderCompiler);
	}

	template <>
	inline std::shared_ptr<RTexture> ResourceManager::load<RTexture>(const std::string& path)
	{
		if (path.empty())
		{
			return nullptr;
		}

		return textureCache.load(m_filesystemModule->getEngineAbsolutePath(path));
	}

	template <>
	inline void ResourceManager::unload<RMesh>(const std::string& path)
	{
		meshCache.unload<RMesh>(m_filesystemModule->getEngineAbsolutePath(path));
	}

	template <>
	inline void ResourceManager::unload<RShader>(const std::string& path)
	{
		shaderCache.unload<RShader>(m_filesystemModule->getEngineAbsolutePath(path));
	}

	template <>
	inline void ResourceManager::unload<RTexture>(const std::string& path)
	{
		textureCache.unload<RTexture>(m_filesystemModule->getEngineAbsolutePath(path));
	}
}
