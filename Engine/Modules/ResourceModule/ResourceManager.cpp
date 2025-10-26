#include "ResourceManager.h"
#include "ResourceManager.h"

#include "../FilesystemModule/FilesystemModule.h"
#include "../ShaderCompilerModule/ShaderCompiler.h"
#include "../LoggerModule/Logger.h"

#include "ResourceCache.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "../../EngineContext/CoreGlobal.h"

namespace ResourceModule
{
    void ResourceManager::startup()
    {
        m_logger = GCM(Logger::Logger);
        m_filesystemModule = GCM(FilesystemModule::FilesystemModule);
            
        m_shaderCompiler = GCM(ShaderCompiler::ShaderCompiler);

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
