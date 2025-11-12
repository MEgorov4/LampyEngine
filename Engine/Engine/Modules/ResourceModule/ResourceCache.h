#pragma once
#include <EngineMinimal.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <shared_mutex>

namespace ResourceModule
{
template <typename T> class ResourceCache
{
  public:
    std::shared_ptr<T> find(const AssetID &guid) const noexcept
    {
        if (guid.empty())
            return nullptr;
        
        std::shared_lock lock(m_mutex);
        if (auto it = m_cache.find(guid); it != m_cache.end())
            return it->second.lock();
        return nullptr;
    }

    void put(const AssetID &guid, std::shared_ptr<T> resource)
    {
        LT_ASSERT_MSG(!guid.empty(), "Cannot put resource with empty GUID");
        LT_ASSERT_MSG(resource, "Cannot put null resource into cache");
        
        std::unique_lock lock(m_mutex);
        m_cache[guid] = resource;
    }

    void remove(const AssetID &guid)
    {
        if (guid.empty())
            return;
        
        std::unique_lock lock(m_mutex);
        m_cache.erase(guid);
    }

    void removeUnused()
    {
        std::unique_lock lock(m_mutex);
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
        std::unique_lock lock(m_mutex);
        m_cache.clear();
    }

    size_t size() const
    {
        std::shared_lock lock(m_mutex);
        return m_cache.size();
    }

  private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<AssetID, std::weak_ptr<T>, AssetID::Hasher> m_cache;
};
} // namespace ResourceModule
