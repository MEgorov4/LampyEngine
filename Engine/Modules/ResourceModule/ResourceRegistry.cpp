#include "ResourceRegistry.h"

void ResourceRegistry::registerResource(const std::string& guid, std::shared_ptr<BaseResource> resource)
{
	registry[guid] = resource;
}

void ResourceRegistry::unregisterResourceByGUID(const std::string& guid)
{
	auto it = registry.find(guid);
	if (it != registry.end())
	{
		registry.erase(it);
	}
}

std::shared_ptr<BaseResource> ResourceRegistry::getResource(const std::string& guid) const
{
	auto it = registry.find(guid);
	return (it != registry.end()) ? it->second : nullptr;
}
