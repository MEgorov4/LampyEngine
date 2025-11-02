#pragma once
#include <EngineMinimal.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include "../../EngineContext/Foundation/Assert/Assert.h"

namespace ResourceModule
{
template <typename T> class ResourceCache
{
  public:
    std::shared_ptr<T> find(const AssetID &guid) const noexcept
    {
        // Пустой AssetID допустим для опциональных ресурсов - возвращаем nullptr
        if (guid.empty())
            return nullptr;
        
        if (auto it = m_cache.find(guid); it != m_cache.end())
            return it->second.lock();
        return nullptr;
    }

    void put(const AssetID &guid, std::shared_ptr<T> resource)
    {
        // Пустой GUID недопустим при регистрации ресурса в кэше
        LT_ASSERT_MSG(!guid.empty(), "Cannot put resource with empty GUID");
        LT_ASSERT_MSG(resource, "Cannot put null resource into cache");
        
        m_cache[guid] = resource;
    }

    void removeUnused()
    {
        for (auto it = m_cache.begin(); it != m_cache.end();)
        {
            if (it->second.expired())
                it = m_cache.erase(it);
            else
                ++it;
        }
    }

    void clear()
    {
        m_cache.clear();
    }

  private:
    std::unordered_map<AssetID, std::weak_ptr<T>, AssetID::Hasher> m_cache;
};
} // namespace ResourceModule
