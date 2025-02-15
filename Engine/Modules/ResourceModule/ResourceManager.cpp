#include "ResourceManager.h"

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

void ResourceManager::clearAllCache()
{
	meshCache.clear();
	shaderCache.clear();
	textureCache.clear();
}

ResourceManager::ResourceManager()
{
	meshCache = MeshCache();
	shaderCache = ShaderCache();
	textureCache = TextureCache();
}
