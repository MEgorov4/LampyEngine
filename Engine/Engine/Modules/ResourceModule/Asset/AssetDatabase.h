#pragma once
#include "AssetInfo.h"
#include "Foundation/Memory/ResourceAllocator.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

using EngineCore::Foundation::ResourceAllocator;

namespace ResourceModule
{
class AssetDatabase
{
  public:
    bool load(const std::string& path);
    bool save(const std::string& path) const;

    template <typename Fn> void forEach(const Fn& fn) const;

    template <typename Fn> void forEachByOrigin(AssetOrigin origin, const Fn& fn) const;

    void upsert(const AssetInfo& info);
    bool remove(const AssetID& guid);
    std::optional<AssetInfo> get(const AssetID& guid) const;
    std::optional<AssetInfo> findBySource(const std::string& srcPath) const;

    std::vector<AssetInfo, ResourceAllocator<AssetInfo>> getByOrigin(AssetOrigin origin) const;

    void clear();
    size_t size() const;

  private:
    std::unordered_map<AssetID, AssetInfo, AssetID::Hasher> m_assets;
    std::unordered_map<std::string, AssetID> m_sourceToGuid;
    mutable std::shared_mutex m_mutex;
};

template <typename Fn> inline void AssetDatabase::forEach(const Fn& fn) const
{
    std::shared_lock lock(m_mutex);
    for (const auto& [guid, info] : m_assets)
        fn(guid, info);
}

template <typename Fn> inline void AssetDatabase::forEachByOrigin(AssetOrigin origin, const Fn& fn) const
{
    std::shared_lock lock(m_mutex);
    for (const auto& [guid, info] : m_assets)
        if (info.origin == origin)
            fn(guid, info);
}
} // namespace ResourceModule
