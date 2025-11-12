#pragma once

#include "Asset/AssetDatabase.h"
#include "Asset/AssetID.h"
#include "Mesh.h"
#include "Pak/PakReader.h"
#include "ResourceCache.h"
#include "ResourceRegistry.h"
#include "Foundation/Assert/Assert.h"
#include "Foundation/Memory/MemoryMacros.h"
#include "../../Core/CoreGlobal.h"
#include "../../Foundation/JobSystem/JobSystem.h"

#include <EngineMinimal.h>
#include <functional>

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
    std::unique_ptr<PakReader, std::function<void(PakReader*)>> m_pakReader;
    ResourceRegistry m_registry;
    AssetDatabase *m_assetDatabase = nullptr;
    std::filesystem::path m_engineResourcesRoot;
    std::filesystem::path m_projectResourcesRoot;
    bool m_usePak = false;
    bool m_periodicCleanupEnabled = false;

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
    void cleanupExpiredCacheEntries();
    
    EngineCore::Foundation::JobHandle scheduleCleanupExpiredJob();

    void setDatabase(AssetDatabase *db) noexcept
    {
        LT_ASSERT_MSG(db, "Cannot set null AssetDatabase");
        m_assetDatabase = db;
        LT_LOGI("ResourceManager", "AssetDatabase connected");
    }
    AssetDatabase *getDatabase() noexcept
    {
        return m_assetDatabase;
    }

    void setEngineResourcesRoot(const std::filesystem::path &path) noexcept
    {
        LT_ASSERT_MSG(!path.empty(), "Engine resources root path cannot be empty");
        m_engineResourcesRoot = path;
        LT_LOGI("ResourceManager", "Engine resources root set: " + path.string());
    }
    void setProjectResourcesRoot(const std::filesystem::path &path) noexcept
    {
        LT_ASSERT_MSG(!path.empty(), "Project resources root path cannot be empty");
        m_projectResourcesRoot = path;
        LT_LOGI("ResourceManager", "Project resources root set: " + path.string());
    }

    template <typename T> std::shared_ptr<T> load(const AssetID &id);
    template <typename T> std::shared_ptr<T> loadBySource(const std::string &path);
};

} // namespace ResourceModule

namespace ResourceModule
{
template <typename T> std::shared_ptr<T> ResourceManager::load(const AssetID &id)
{
    if (id.empty())
        return nullptr;
    
    auto &cache = getCache<T>();
    if (auto cached = cache.find(id))
    {
        LT_LOGI("ResourceManager", std::format("Resource [{}] found in cache", id.str()));
        return cached;
    }

    if (!m_assetDatabase)
    {
        LT_LOGE("ResourceManager", "AssetDatabase not set! Call setDatabase() first");
        return nullptr;
    }
    
    LT_LOGI("ResourceManager", std::format("Loading resource [{}]...", id.str()));
    
    auto infoOpt = m_assetDatabase->get(id);
    if (!infoOpt)
    {
        LT_LOGW("ResourceManager", "AssetInfo not found for GUID: " + id.str() + ", trying fallback search by path");
        
        return nullptr;
    }

    const auto &info = *infoOpt;
    std::filesystem::path sourcePath;

    if (m_usePak && m_pakReader && m_pakReader->exists(id))
    {
        LT_LOGI("ResourceManager", std::format("Loading resource [{}] from PAK", id.str()));
        auto data = m_pakReader->readAsset(id);
        if (!data)
        {
            LT_LOGE("ResourceManager", "Failed to read asset from pak: " + id.str());
            return nullptr;
        }
        
        if (data->empty())
        {
            LT_LOGE("ResourceManager", "PAK data is empty for asset: " + id.str());
            return nullptr;
        }
        
        LT_LOGI("ResourceManager", std::format("Read {} bytes from PAK for [{}]", data->size(), id.str()));

        std::filesystem::path tmp = std::filesystem::temp_directory_path() / (id.str() + ".tmp");
        std::ofstream ofs(tmp, std::ios::binary);
        if (!ofs.is_open())
        {
            LT_LOGE("ResourceManager", "Failed to create temporary file for PAK asset: " + tmp.string());
            return nullptr;
        }
        
        ofs.write(reinterpret_cast<const char *>(data->data()), data->size());
        if (ofs.fail())
        {
            LT_LOGE("ResourceManager", "Failed to write temporary file for PAK asset: " + tmp.string());
            ofs.close();
            std::error_code ec;
            std::filesystem::remove(tmp, ec);
            if (ec)
                LT_LOGW("ResourceManager", "Failed to remove temporary file after write error: " + ec.message());
            return nullptr;
        }
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
        LT_LOGI("ResourceManager", std::format("Loading resource [{}] from filesystem: {}", id.str(), sourcePath.string()));
    }

    std::shared_ptr<T> resource;
    bool isTempFile = (sourcePath.extension() == ".tmp");
    try
    {
        using namespace EngineCore::Foundation;
        // Allocate memory for resource using MemorySystem
        void* resourceMemory = AllocateMemory(sizeof(T), alignof(T), MemoryTag::Resource);
        if (!resourceMemory)
        {
            LT_LOGE("ResourceManager", "Failed to allocate memory for resource: " + id.str());
            if (isTempFile)
            {
                std::error_code ec;
                std::filesystem::remove(sourcePath, ec);
                if (ec)
                    LT_LOGW("ResourceManager", "Failed to remove temporary file: " + ec.message());
            }
            return nullptr;
        }
        
        // Construct resource using placement new
        T* resourcePtr = nullptr;
        try
        {
            resourcePtr = new(resourceMemory) T(sourcePath.string());
        }
        catch (...)
        {
            // If construction fails, deallocate memory and rethrow
            DeallocateMemory(resourceMemory, MemoryTag::Resource);
            throw;
        }
        
        // Create shared_ptr with custom deleter
        resource = std::shared_ptr<T>(resourcePtr, [](T* p) {
            if (p)
            {
                p->~T();
                DeallocateMemory(p, MemoryTag::Resource);
            }
        });
        
        LT_LOGI("ResourceManager", std::format("Resource [{}] created successfully", id.str()));
    }
    catch (const std::exception &e)
    {
        LT_LOGE("ResourceManager", std::string("Failed to construct resource: ") + e.what());
        if (isTempFile)
        {
            std::error_code ec;
            std::filesystem::remove(sourcePath, ec);
            if (ec)
                LT_LOGW("ResourceManager", "Failed to remove temporary file after exception: " + ec.message());
        }
        return nullptr;
    }
    catch (...)
    {
        LT_LOGE("ResourceManager", "Unknown exception while constructing resource: " + id.str());
        if (isTempFile)
        {
            std::error_code ec;
            std::filesystem::remove(sourcePath, ec);
            if (ec)
                LT_LOGW("ResourceManager", "Failed to remove temporary file after unknown exception: " + ec.message());
        }
        return nullptr;
    }

    // Удаляем временный файл после успешной загрузки ресурса
    // Ресурс уже прочитал все данные в память
    if (isTempFile)
    {
        std::error_code ec;
        std::filesystem::remove(sourcePath, ec);
        if (ec)
        {
            LT_LOGW("ResourceManager", "Failed to remove temporary file after loading: " + ec.message() + 
                     " (file: " + sourcePath.string() + ")");
        }
    }

    cache.put(id, resource);
    m_registry.registerResource(id, resource);
    LT_LOGI("ResourceManager", std::format("Resource [{}] registered in cache and registry", id.str()));
    return resource;
}

template <typename T> std::shared_ptr<T> ResourceManager::loadBySource(const std::string &path)
{
    LT_ASSERT_MSG(!path.empty(), "Source path cannot be empty");
    LT_ASSERT_MSG(m_assetDatabase, "AssetDatabase not set for loadBySource()");

    LT_LOGI("ResourceManager", std::format("Loading resource by source path: {}", path));
    auto infoOpt = m_assetDatabase->findBySource(path);
    if (!infoOpt)
    {
        LT_LOGW("ResourceManager", "Asset not found in database for source: " + path);
        return nullptr;
    }

    LT_ASSERT_MSG(!infoOpt->guid.empty(), "Found AssetInfo has empty GUID");
    LT_LOGI("ResourceManager", std::format("Found AssetID [{}] for source path: {}", infoOpt->guid.str(), path));
    return load<T>(infoOpt->guid);
}

} // namespace ResourceModule
