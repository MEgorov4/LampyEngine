#pragma once

#include "Asset/AssetDatabase.h"
#include "Asset/AssetID.h"
#include "Mesh.h"
#include "Pak/PakReader.h"
#include "ResourceCache.h"
#include "ResourceRegistry.h"
#include "../../EngineContext/Foundation/Assert/Assert.h"

#include <EngineMinimal.h>

namespace ResourceModule
{

class AssetRegistryAccessor
{
    inline static AssetDatabase *s_database = nullptr;

  public:
    static void Set(AssetDatabase *db) noexcept
    {
        LT_ASSERT_MSG(db, "Cannot set null AssetDatabase");
        s_database = db;
    }

    static AssetDatabase *Get() noexcept
    {
        LT_ASSERT_MSG(s_database, "AssetDatabase not set!");
        return s_database;
    }
};

class ResourceManager : public IModule
{
    std::unique_ptr<PakReader> m_pakReader;
    ResourceRegistry m_registry;
    AssetDatabase *m_assetDatabase = nullptr;
    std::filesystem::path m_engineResourcesRoot;
    std::filesystem::path m_projectResourcesRoot;
    bool m_usePak = false;

    template <typename T> ResourceCache<T> &getCache()
    {
        static ResourceCache<T> cache;
        return cache;
    }

  public:
    void startup() override;
    void shutdown() override;

    void mountPak(const std::string &mountPak);
    void unload(const AssetID &id);
    void clearAll();

    // 🔧 Новые методы для связи
    void setDatabase(AssetDatabase *db) noexcept
    {
        LT_ASSERT_MSG(db, "Cannot set null AssetDatabase");
        m_assetDatabase = db;
    }
    AssetDatabase *getDatabase() noexcept
    {
        return m_assetDatabase;
    }

    void setEngineResourcesRoot(const std::filesystem::path &path) noexcept
    {
        LT_ASSERT_MSG(!path.empty(), "Engine resources root path cannot be empty");
        m_engineResourcesRoot = path;
    }
    void setProjectResourcesRoot(const std::filesystem::path &path) noexcept
    {
        LT_ASSERT_MSG(!path.empty(), "Project resources root path cannot be empty");
        m_projectResourcesRoot = path;
    }

    template <typename T> std::shared_ptr<T> load(const AssetID &id);
    template <typename T> std::shared_ptr<T> loadBySource(const std::string &path);
};

} // namespace ResourceModule

namespace ResourceModule
{
template <typename T> std::shared_ptr<T> ResourceManager::load(const AssetID &id)
{
    LT_ASSERT_MSG(!id.str().empty(), "AssetID cannot be empty");
    
    auto &cache = getCache<T>();
    if (auto cached = cache.find(id))
        return cached;

    LT_ASSERT_MSG(m_assetDatabase, "AssetDatabase not set! Call setDatabase() first");
    
    auto infoOpt = m_assetDatabase->get(id);
    if (!infoOpt)
    {
        LT_LOGE("ResourceManager", "AssetInfo not found for GUID: " + id.str());
        return nullptr;
    }

    const auto &info = *infoOpt;
    std::filesystem::path sourcePath;

    if (m_usePak && m_pakReader && m_pakReader->exists(id))
    {
        auto data = m_pakReader->readAsset(id);
        if (!data)
        {
            LT_LOGE("ResourceManager", "Failed to read asset from pak: " + id.str());
            return nullptr;
        }
        
        LT_ASSERT_MSG(!data->empty(), "PAK data is empty for asset: " + id.str());

        // временный файл
        std::filesystem::path tmp = std::filesystem::temp_directory_path() / (id.str() + ".tmp");
        std::ofstream ofs(tmp, std::ios::binary);
        LT_ASSERT_MSG(ofs.is_open(), "Failed to create temporary file for PAK asset");
        
        ofs.write(reinterpret_cast<const char *>(data->data()), data->size());
        ofs.close();

        sourcePath = tmp;
    }
    else
    {
        sourcePath = info.importedPath;
        if (!std::filesystem::exists(sourcePath))
        {
            LT_LOGE("ResourceManager", "Missing imported file: " + sourcePath.string());
            return nullptr;
        }
    }

    // 🔧 Создаём ресурс через его конструктор(path)
    std::shared_ptr<T> resource;
    try
    {
        resource = std::make_shared<T>(sourcePath.string());
        LT_ASSERT_MSG(resource, "Failed to create resource instance");
    }
    catch (const std::exception &e)
    {
        LT_LOGE("ResourceManager", std::string("Failed to construct resource: ") + e.what());
        if (sourcePath.extension() == ".tmp")
            std::filesystem::remove(sourcePath);
        return nullptr;
    }

    if (sourcePath.extension() == ".tmp")
        std::filesystem::remove(sourcePath);

    cache.put(id, resource);
    m_registry.registerResource(id, resource);
    return resource;
}

template <typename T> std::shared_ptr<T> ResourceManager::loadBySource(const std::string &path)
{
    LT_ASSERT_MSG(!path.empty(), "Source path cannot be empty");
    LT_ASSERT_MSG(m_assetDatabase, "AssetDatabase not set for loadBySource()");

    auto infoOpt = m_assetDatabase->findBySource(path);
    if (!infoOpt)
    {
        LT_LOGW("ResourceManager", "Asset not found in database for source: " + path);
        return nullptr;
    }

    LT_ASSERT_MSG(!infoOpt->guid.str().empty(), "Found AssetInfo has empty GUID");
    return load<T>(infoOpt->guid);
}

} // namespace ResourceModule
