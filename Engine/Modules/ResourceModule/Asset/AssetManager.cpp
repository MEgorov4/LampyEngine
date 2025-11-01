#include "AssetManager.h"

#include "Importers/MeshImporter.h"
#include "Importers/ShaderImporter.h"
#include "Importers/TextureImporter.h"
#include "Importers/WorldImporter.h"

using namespace ResourceModule;

namespace
{
class AssetFileListener : public efsw::FileWatchListener
{
  public:
    explicit AssetFileListener(AssetManager* mgr) : m_mgr(mgr)
    {
    }

    void handleFileAction(efsw::WatchID, const std::string& dir, const std::string& filename, efsw::Action action,
                          std::string /*oldFilename*/) override
    {
        LT_PROFILE_SCOPE("AssetManager::AssetFileListener::handleFileAction");
        std::filesystem::path fullPath = std::filesystem::path(dir) / filename;
        m_mgr->handleFileAction(fullPath.string(), action);
    }

  private:
    AssetManager* m_mgr;
};
} // namespace

// --------------------------------------------------------

void AssetManager::startup()
{
    LT_PROFILE_SCOPE("AssetManager::startup");
    LT_LOGI("AssetManager", "Starting up AssetManager...");

    if (m_projectResourcesRoot.empty() || m_engineResourcesRoot.empty())
    {
        LT_LOGE("AssetManager", "Resource roots not configured!");
        return;
    }

    // Загружаем базу ассетов
    m_database.load(m_dbPath.string());

    // Настраиваем file watcher
    m_watcher  = std::make_unique<efsw::FileWatcher>();
    m_listener = std::make_unique<AssetFileListener>(this);

    registerDefaultImporters();

    // Следим за обоими каталогами
    watchDirectory(m_engineResourcesRoot.string());
    watchDirectory(m_projectResourcesRoot.string());

    // Импортируем ресурсы
    LT_LOGI("AssetManager", "Scanning engine resources...");
    scanAndImportAllIn(m_engineResourcesRoot);

    LT_LOGI("AssetManager", "Scanning project resources...");
    scanAndImportAllIn(m_projectResourcesRoot);

    LT_LOGI("AssetManager", "Watching: " + m_engineResourcesRoot.string());
    LT_LOGI("AssetManager", "Watching: " + m_projectResourcesRoot.string());
}

// --------------------------------------------------------

void AssetManager::shutdown()
{
    LT_PROFILE_SCOPE("AssetManager::shudown");
    LT_LOGI("AssetManager", "Shutting down AssetManager...");
    saveDatabase();

    if (m_watchThread && m_watchThread->joinable())
        m_watchThread->join();
}

// --------------------------------------------------------

void AssetManager::registerDefaultImporters()
{
    LT_PROFILE_SCOPE("AssetManager::registerDefaultImporters");
    m_importers.registerImporter(std::make_unique<TextureImporter>());
    m_importers.registerImporter(std::make_unique<ShaderImporter>());
    m_importers.registerImporter(std::make_unique<MeshImporter>());
    m_importers.registerImporter(std::make_unique<WorldImporter>());
}

// --------------------------------------------------------

void AssetManager::watchDirectory(const std::string& path)
{
    m_watcher->addWatch(path, m_listener.get(), true);
    if (!m_watchThread)
        m_watchThread = std::make_unique<std::thread>(
            [this]()
            {
                LT_PROFILE_SCOPE("AssetManager::watchDirectory");
                m_watcher->watch();
            });
}

// --------------------------------------------------------

void AssetManager::handleFileAction(const std::string& fullPath, efsw::Action action)
{
    LT_PROFILE_SCOPE("AssetManager::handleFileAction");
    std::scoped_lock lock(m_queueMutex);
    if (action == efsw::Actions::Modified || action == efsw::Actions::Add)
        m_changedFiles.push_back(fullPath);
}

// --------------------------------------------------------

void AssetManager::processFileChanges()
{
    LT_PROFILE_SCOPE("AssetManager::processFileChanges");
    std::vector<std::string> files;
    {
        std::scoped_lock lock(m_queueMutex);
        files.swap(m_changedFiles);
    }

    for (const auto& f : files)
    {
        std::string ext = std::filesystem::path(f).extension().string();
        if (auto* importer = m_importers.findImporter(ext))
        {
            LT_LOGI("AssetManager", "Reimport: " + f);

            std::filesystem::path full = f;
            std::filesystem::path rel;

            if (full.string().starts_with(m_projectResourcesRoot.string()))
                rel = std::filesystem::relative(full, m_projectResourcesRoot);
            else if (full.string().starts_with(m_engineResourcesRoot.string()))
                rel = std::filesystem::relative(full, m_engineResourcesRoot);
            else
                rel = full;

            auto info       = importer->import(full, m_cacheRoot);
            info.sourcePath = rel.generic_string();
            info.guid       = MakeDeterministicIDFromPath(info.sourcePath);

            m_database.upsert(info);
        }
        else
        {
            LT_LOGW("AssetManager", "No importer for extension: " + ext);
        }
    }
}

// --------------------------------------------------------

void AssetManager::scanAndImportAllIn(const std::filesystem::path& root)
{
    LT_PROFILE_SCOPE("AssetManager::scanAndImportAllIn");
    if (!std::filesystem::exists(root))
    {
        LT_LOGE("AssetManager", "Directory does not exist: " + root.string());
        return;
    }

    for (auto& entry : std::filesystem::recursive_directory_iterator(root))
    {
        auto importer = m_importers.findImporter(entry.path().extension().string());
        if (!importer)
            continue;

        // --- Добавляем нормализацию пути ---
        std::filesystem::path abs = std::filesystem::weakly_canonical(entry.path());
        std::filesystem::path rel;
        AssetOrigin origin = AssetOrigin::Project;

        if (!m_projectResourcesRoot.empty() && abs.string().starts_with(m_projectResourcesRoot.string()))
        {
            rel    = std::filesystem::relative(abs, m_projectResourcesRoot);
            origin = AssetOrigin::Project;
        }
        else if (!m_engineResourcesRoot.empty() && abs.string().starts_with(m_engineResourcesRoot.string()))
        {
            rel    = std::filesystem::relative(abs, m_engineResourcesRoot);
            origin = AssetOrigin::Engine;
        }
        else
        {
            rel = abs.filename();
        }

        // --- Импорт ---
        auto info = importer->import(abs, m_cacheRoot);

        // --- Перезаписываем корректные поля ---
        info.sourcePath = rel.generic_string();
        info.guid       = MakeDeterministicIDFromPath(info.sourcePath);
        info.origin     = origin;

        m_database.upsert(info);

        LT_LOGI("AssetManager", std::format("Imported [{}] {}", info.guid.str(), info.sourcePath));
    }
}

// --------------------------------------------------------

void AssetManager::scanAndImportAll()
{
    LT_PROFILE_SCOPE("AssetManager::scanAndImportAll");
    scanAndImportAllIn(m_engineResourcesRoot);
    scanAndImportAllIn(m_projectResourcesRoot);
}

// --------------------------------------------------------

void AssetManager::saveDatabase()
{
    LT_PROFILE_SCOPE("AssetManager::saveDatabase");
    m_database.save(m_dbPath.string());
}
