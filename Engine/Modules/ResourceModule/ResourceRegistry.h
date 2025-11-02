#pragma once
#include <stddef.h>
#include <unordered_map>
#include <memory>
#include <shared_mutex>
#include <list>
#include <mutex>
#include <type_traits>
#include <utility>

#include "Asset/AssetID.h"
#include "BaseResource.h"
#include "../../EngineContext/Foundation/Assert/Assert.h"

namespace ResourceModule
{
class BaseResource;

    class ResourceRegistry
    {
    public:
        void registerResource(const AssetID& guid, std::shared_ptr<BaseResource> resource)
        {
            LT_ASSERT_MSG(!guid.str().empty(), "Cannot register resource with empty GUID");
            LT_ASSERT_MSG(resource, "Cannot register null resource");
            
            std::unique_lock lock(m_mutex);
            m_resources[guid] = std::move(resource);
        }

        void unregister(const AssetID& guid)
        {
            LT_ASSERT_MSG(!guid.str().empty(), "Cannot unregister resource with empty GUID");
            
            std::unique_lock lock(m_mutex);
            m_resources.erase(guid);
        }

        std::shared_ptr<BaseResource> get(const AssetID& guid) const
        {
            LT_ASSERT_MSG(!guid.str().empty(), "Cannot get resource with empty GUID");
            
            std::shared_lock lock(m_mutex);
            if (auto it = m_resources.find(guid); it != m_resources.end())
                return it->second;
            return nullptr;
        }

        void clear()
        {
            std::unique_lock lock(m_mutex);
            m_resources.clear();
        }

        size_t size() const
        {
            std::shared_lock lock(m_mutex);
            return m_resources.size();
        }

    private:
        mutable std::shared_mutex m_mutex;
        std::unordered_map<AssetID, std::shared_ptr<BaseResource>, AssetID::Hasher> m_resources;
    };
}
