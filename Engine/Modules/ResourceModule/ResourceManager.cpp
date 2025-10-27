#include "ResourceManager.h"

#include <Modules/ShaderCompilerModule/ShaderCompiler.h>

#include "ResourceCache.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"

namespace ResourceModule
{
    void ResourceManager::startup()
    {
            
        m_shaderCompiler = GCM(ShaderCompiler::ShaderCompiler);
        
        LT_LOGI("ResourceManager", "Startup");
        LT_LOGI("ResourceManager", "Create resource caches");
        
        meshCache = ResourceCache<RMesh>();
        shaderCache = ResourceCache<RShader>();
        textureCache = ResourceCache<RTexture>();
    }

    void ResourceManager::shutdown()
    {
        LT_LOGI("ResourceManager", "Shutdown");
        clearAllCache();
    }

    void ResourceManager::clearAllCache()
    {
        LT_LOGI("ResourceManager", "Clear resource caches");
        meshCache.clear<RMesh>();
        shaderCache.clear<RShader>();
        textureCache.clear<RTexture>();
    }


}
