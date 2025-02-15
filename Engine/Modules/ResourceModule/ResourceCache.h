#pragma once

#include <string>
#include <memory>
#include <unordered_map>

template<typename T>
class ResourceCache
{
public:
	std::shared_ptr<T> load(const std::string& path);
	void unload(const std::string& path);
	void clear();
	void removeUnused();
private:
	std::unordered_map<std::string, std::shared_ptr<T>> cache;
};



template<typename T>
inline std::shared_ptr<T> ResourceCache<T>::load(const std::string& path)
{
	auto it = cache.find(path);
	if (it != cache.end() && it->second)
	{
		return it->second;
	}

	auto resource = std::make_shared<T>(path);
	cache[path] = resource;

	return resource;
}

template<typename T>
inline void ResourceCache<T>::unload(const std::string& path)
{
	auto it = cache.find(path);
	if (it != cache.end())
	{
		cache.erase(it);
	}
}

template<typename T>
inline void ResourceCache<T>::clear()
{
	cache.clear();
}

template<typename T>
inline void ResourceCache<T>::removeUnused()
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
