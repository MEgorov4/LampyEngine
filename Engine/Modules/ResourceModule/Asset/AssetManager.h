#pragma once

#include "AssetDatabase.h"
#include "AssetID.h"
#include "AssetImporterHub.h"

#include <EngineMinimal.h>
#include <efsw/efsw.hpp>
#include <filesystem>
#include <mutex>
#include <thread>
#include <vector>

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

    void registerDefaultImporters();
    void watchDirectory(const std::string& path);

    AssetDatabase& getDatabase() noexcept
    {
        return m_database;
    }

    void handleFileAction(const std::string& fullPath, efsw::Action action);

    void setProjectResourcesRoot(const std::filesystem::path& root) noexcept
    {
        m_projectResourcesRoot = root;
    }
    void setEngineResourcesRoot(const std::filesystem::path& root) noexcept
    {
        m_engineResourcesRoot = root;
    }
    void setCacheRoot(const std::filesystem::path& root) noexcept
    {
        m_cacheRoot = root;
    }
    void setDatabasePath(const std::filesystem::path& path) noexcept
    {
        m_dbPath = path;
    }

  private:
    std::unique_ptr<efsw::FileWatcher> m_watcher;
    std::unique_ptr<efsw::FileWatchListener> m_listener;
    std::unique_ptr<std::thread> m_watchThread;
    std::mutex m_queueMutex;
    std::vector<std::string> m_changedFiles;

    AssetImporterHub m_importers;
    AssetDatabase m_database;

    std::filesystem::path m_engineResourcesRoot;
    std::filesystem::path m_projectResourcesRoot;
    std::filesystem::path m_cacheRoot;
    std::filesystem::path m_dbPath;
};
} // namespace ResourceModule
