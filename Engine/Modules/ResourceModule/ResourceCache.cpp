#include "ResourceCache.h"

#include "../MemoryModule/PoolAllocator.h"

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

#include "../LoggerModule/Logger.h"

template<typename T>
ResourceCache<T>::ResourceCache(size_t objectSize, size_t objectNum, void* placement)
{
	if (placement)
	{
		m_allocator = PoolAllocator(objectSize, objectNum, placement);
	}
	else
	{
		m_allocator = PoolAllocator(objectSize, objectNum);
	}
}

template<typename T>
std::shared_ptr<T> ResourceCache<T>::load(const std::string& path)
{
	auto it = cache.find(path);
	if (it != cache.end() && it->second)
	{
		LOG_INFO("ResourceModule: Using resource in " + path);
		return it->second;
	}
	
	T* resource = static_cast<T*>(m_allocator.allocate());
	new (resource) T(path);

	auto shared_resource = std::shared_ptr<T>(
		resource,
		[allocator = &m_allocator](T* ptr) {
			ptr->~T();
			allocator->deallocate(ptr);
		});

	// auto resource = std::make_shared<T>(path);
	cache[path] = shared_resource;

	LOG_INFO("ResourceModule: Created resource in " + path + "\n");
	return shared_resource;
}

template<typename T>
void ResourceCache<T>::unload(const std::string& path)
{
	auto it = cache.find(path);
	if (it != cache.end())
	{
		LOG_INFO("ResourceModule: Unload resource in " + path + "\n");
		if (it->second.use_count() == 1)
		{
			it->second.reset();
		}
		cache.erase(it);
	}
}

template<typename T>
void ResourceCache<T>::clear()
{
	cache.clear();
}

template<typename T>
void ResourceCache<T>::removeUnused()
{
	for (auto it = cache.begin(); it != cache.end();)
	{
		if (it->second.use_count() == 1)
		{
			it = cache.erase(it);
		}
		else
		{
			++it;
		}
	}
}

template class ResourceCache<RMaterial>;
template class ResourceCache<RTexture>;
template class ResourceCache<RShader>;
template class ResourceCache<RMesh>;