#include "ResourceManager.h"
#include "ResourceManager.h"

#include "../FilesystemModule/FilesystemModule.h"
#include "../ShaderCompilerModule/ShaderCompiler.h"
#include "../LoggerModule/Logger.h"

#include "ResourceCache.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"

namespace ResourceModule
{
    void ResourceManager::startup(const ModuleRegistry& registry)
    {
        m_logger = std::dynamic_pointer_cast<Logger::Logger>(registry.getModule("Logger"));
        m_filesystemModule = std::dynamic_pointer_cast<FilesystemModule::FilesystemModule>(
            registry.getModule("FilesystemModule"));
        m_shaderCompiler = std::dynamic_pointer_cast<ShaderCompiler::ShaderCompiler>(
            registry.getModule("ShaderCompiler"));

        m_logger->log(Logger::LogVerbosity::Info, "Startup", "ResourceManager");
        
        m_logger->log(Logger::LogVerbosity::Info, "Create resource caches", "ResourceManager");
        
        meshCache = ResourceCache<RMesh>();
        shaderCache = ResourceCache<RShader>();
        textureCache = ResourceCache<RTexture>();
    }

    void ResourceManager::shutdown()
    {
        m_logger->log(Logger::LogVerbosity::Info, "Shutdown", "ResourceManager");
        clearAllCache();
    }

    void ResourceManager::clearAllCache()
    {
        m_logger->log(Logger::LogVerbosity::Info, "Clear resource caches", "ResourceManager");
        meshCache.clear<RMesh>();
        shaderCache.clear<RShader>();
        textureCache.clear<RTexture>();
    }


}
