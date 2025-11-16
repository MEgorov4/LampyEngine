#pragma once

#include <EngineMinimal.h>
#include "../../../Foundation/Event/Event.h"
#include "../../../Core/CoreGlobal.h"
#include "../../../Foundation/JobSystem/JobSystem.h"
#include "../../../Foundation/Memory/ResourceAllocator.h"
#include "AssetDatabase.h"
#include "AssetID.h"
#include "AssetImporterHub.h"
#include "AssetWriterHub.h"

#include <efsw/efsw.hpp>
#include <filesystem>
#include <mutex>
#include <thread>
#include <vector>

using EngineCore::Foundation::ResourceAllocator;

namespace ResourceModule
{
class AssetManager : public IModule
{
  public:
    void startup() override;
    void shutdown() override;

    void scanAndImportAllIn(const std::filesystem::path& root);
    void scanAndImportAll();
    void processFileChanges();
    void saveDatabase();
    
    EngineCore::Foundation::JobHandle scheduleRescanJob();

    void registerDefaultImporters();
    void registerDefaultWriters();
    void watchDirectory(const std::string& path);

    AssetWriterHub& getWriterHub() noexcept
    {
        return m_writers;
    }

    const AssetWriterHub& getWriterHub() const noexcept
    {
        return m_writers;
    }

    AssetDatabase& getDatabase() noexcept
    {
        return m_database;
    }

    void handleFileAction(const std::string& fullPath, efsw::Action action);

    void setProjectResourcesRoot(const std::filesystem::path& root) noexcept
    {
        LT_ASSERT_MSG(!root.empty(), "Project resources root path cannot be empty");
        m_projectResourcesRoot = root;
    }
    void setEngineResourcesRoot(const std::filesystem::path& root) noexcept
    {
        LT_ASSERT_MSG(!root.empty(), "Engine resources root path cannot be empty");
        m_engineResourcesRoot = root;
    }
    void setCacheRoot(const std::filesystem::path& root) noexcept
    {
        LT_ASSERT_MSG(!root.empty(), "Cache root path cannot be empty");
        m_cacheRoot = root;
    }
    void setDatabasePath(const std::filesystem::path& path) noexcept
    {
        LT_ASSERT_MSG(!path.empty(), "Database path cannot be empty");
        m_dbPath = path;
    }

    EngineCore::Foundation::Event<const AssetInfo&> OnAssetImported;

  private:
    std::unique_ptr<efsw::FileWatcher> m_watcher;
    std::unique_ptr<efsw::FileWatchListener> m_listener;
    std::unique_ptr<std::thread> m_watchThread;
    std::mutex m_queueMutex;
    std::vector<std::string, ResourceAllocator<std::string>> m_changedFiles;
    std::vector<efsw::WatchID> m_watchIds;

    AssetImporterHub m_importers;
    AssetWriterHub m_writers;
    AssetDatabase m_database;

    std::filesystem::path m_engineResourcesRoot;
    std::filesystem::path m_projectResourcesRoot;
    std::filesystem::path m_cacheRoot;
    std::filesystem::path m_dbPath;
};
} // namespace ResourceModule
