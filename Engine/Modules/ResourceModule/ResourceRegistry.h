#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace ResourceModule
{
	class BaseResource;

	class ResourceRegistry
	{
	public:
		void registerResource(const std::string& guid, std::shared_ptr<BaseResource> resource);
		void unregisterResourceByGUID(const std::string& guid);

		std::shared_ptr<BaseResource> getResource(const std::string& guid) const;
	private:
		std::unordered_map<std::string, std::shared_ptr<BaseResource>> registry;
	};
}