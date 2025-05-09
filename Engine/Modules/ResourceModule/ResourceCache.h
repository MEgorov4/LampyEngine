#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "../MemoryModule/PoolAllocator.h"

template<typename T>
class ResourceCache
{
public:
	ResourceCache() {};
	ResourceCache(size_t objectSize, size_t objectNum, void* placement = nullptr);

	std::shared_ptr<T> load(const std::string& path);
	void unload(const std::string& path);
	void clear();
	void removeUnused();
private:
	PoolAllocator m_allocator;
	std::unordered_map<std::string, std::shared_ptr<T>> cache;
};