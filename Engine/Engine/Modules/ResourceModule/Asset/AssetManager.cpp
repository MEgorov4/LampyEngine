#include "AssetManager.h"
#include <Foundation/Assert/Assert.h>

#include "Foundation/Profiler/ProfileAllocator.h"
#include "Importers/MeshImporter.h"
#include "Importers/ShaderImporter.h"
#include "Importers/TextureImporter.h"
#include "Importers/WorldImporter.h"
#include "Importers/MaterialImporter.h"

using namespace ResourceModule;

namespace
{
class AssetFileListener : public efsw::FileWatchListener
{
  public:
    explicit AssetFileListener(AssetManager* mgr) : m_mgr(mgr)
    {
        LT_ASSERT_MSG(mgr, "AssetManager pointer cannot be null");
    }

    void handleFileAction(efsw::WatchID, const std::string& dir, const std::string& filename, efsw::Action action,
                          std::string /*oldFilename*/) override
    {
        ZoneScopedN("AssetManager::AssetFileListener::handleFileAction");
        LT_ASSERT_MSG(!dir.empty(), "Directory path cannot be empty");
        LT_ASSERT_MSG(!filename.empty(), "Filename cannot be empty");
        LT_ASSERT_MSG(m_mgr, "AssetManager pointer is null");
        
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
    ZoneScopedN("AssetManager::startup");
    LT_LOGI("AssetManager", "Starting up AssetManager...");

    LT_ASSERT_MSG(!m_projectResourcesRoot.empty(), "Project resources root not configured");
    LT_ASSERT_MSG(!m_engineResourcesRoot.empty(), "Engine resources root not configured");
    LT_ASSERT_MSG(!m_cacheRoot.empty(), "Cache root not configured");
    LT_ASSERT_MSG(!m_dbPath.empty(), "Database path not configured");

    if (m_projectResourcesRoot.empty() || m_engineResourcesRoot.empty())
    {
        LT_LOGE("AssetManager", "Resource roots not configured!");
        return;
    }

    // Загружаем базу ассетов
    LT_LOGI("AssetManager", "Loading asset database from: " + m_dbPath.string());
    bool loaded = m_database.load(m_dbPath.string());
    if (!loaded)
    {
        LT_LOGW("AssetManager", "Failed to load database from: " + m_dbPath.string() + ", starting with empty database");
    }
    else
    {
        LT_LOGI("AssetManager", "Asset database loaded successfully");
    }

    // Настраиваем file watcher
    m_watcher  = std::make_unique<efsw::FileWatcher>();
    LT_ASSERT_MSG(m_watcher, "Failed to create FileWatcher");
    
    m_listener = std::make_unique<AssetFileListener>(this);
    LT_ASSERT_MSG(m_listener, "Failed to create AssetFileListener");

    registerDefaultImporters();

    // Следим за обоими каталогами
    LT_ASSERT_MSG(std::filesystem::exists(m_engineResourcesRoot), "Engine resources root does not exist");
    LT_ASSERT_MSG(std::filesystem::exists(m_projectResourcesRoot), "Project resources root does not exist");
    
    LT_LOGI("AssetManager", "Starting file watchers...");
    watchDirectory(m_engineResourcesRoot.string());
    watchDirectory(m_projectResourcesRoot.string());
    LT_LOGI("AssetManager", "File watchers started");

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
    ZoneScopedN("AssetManager::shudown");
    LT_LOGI("AssetManager", "Shutting down AssetManager...");
    saveDatabase();

    if (m_watchThread && m_watchThread->joinable())
        m_watchThread->join();
}

// --------------------------------------------------------

void AssetManager::registerDefaultImporters()
{
    ZoneScopedN("AssetManager::registerDefaultImporters");
    LT_LOGI("AssetManager", "Registering default asset importers...");
    
    auto textureImporter = std::make_unique<TextureImporter>();
    LT_ASSERT_MSG(textureImporter, "Failed to create TextureImporter");
    m_importers.registerImporter(std::move(textureImporter));
    LT_LOGI("AssetManager", "Registered TextureImporter");
    
    auto shaderImporter = std::make_unique<ShaderImporter>();
    LT_ASSERT_MSG(shaderImporter, "Failed to create ShaderImporter");
    m_importers.registerImporter(std::move(shaderImporter));
    LT_LOGI("AssetManager", "Registered ShaderImporter");
    
    auto meshImporter = std::make_unique<MeshImporter>();
    LT_ASSERT_MSG(meshImporter, "Failed to create MeshImporter");
    m_importers.registerImporter(std::move(meshImporter));
    LT_LOGI("AssetManager", "Registered MeshImporter");
    
    auto worldImporter = std::make_unique<WorldImporter>();
    LT_ASSERT_MSG(worldImporter, "Failed to create WorldImporter");
    m_importers.registerImporter(std::move(worldImporter));
    LT_LOGI("AssetManager", "Registered WorldImporter");
    
    auto materialImporter = std::make_unique<MaterialImporter>();
    LT_ASSERT_MSG(materialImporter, "Failed to create MaterialImporter");
    m_importers.registerImporter(std::move(materialImporter));
    LT_LOGI("AssetManager", "Registered MaterialImporter");
    
    LT_LOGI("AssetManager", "All default importers registered");
}

// --------------------------------------------------------

void AssetManager::watchDirectory(const std::string& path)
{
    LT_ASSERT_MSG(!path.empty(), "Watch directory path cannot be empty");
    LT_ASSERT_MSG(m_watcher, "FileWatcher is null");
    LT_ASSERT_MSG(m_listener, "FileWatchListener is null");
    LT_ASSERT_MSG(std::filesystem::exists(path), "Watch directory does not exist: " + path);
    
    m_watcher->addWatch(path, m_listener.get(), true);
    if (!m_watchThread)
        m_watchThread = std::make_unique<std::thread>(
            [this]()
            {
                ZoneScopedN("AssetManager::watchDirectory");
                LT_ASSERT_MSG(m_watcher, "FileWatcher is null in watch thread");
                m_watcher->watch();
            });
    LT_ASSERT_MSG(m_watchThread, "Failed to create watch thread");
}

// --------------------------------------------------------

void AssetManager::handleFileAction(const std::string& fullPath, efsw::Action action)
{
    ZoneScopedN("AssetManager::handleFileAction");
    LT_ASSERT_MSG(!fullPath.empty(), "File path cannot be empty");
    
    std::scoped_lock lock(m_queueMutex);
    if (action == efsw::Actions::Modified || action == efsw::Actions::Add)
        m_changedFiles.push_back(fullPath);
}

// --------------------------------------------------------

void AssetManager::processFileChanges()
{
    ZoneScopedN("AssetManager::processFileChanges");
    std::vector<std::string, ProfileAllocator<std::string>> files;
    {
        std::scoped_lock lock(m_queueMutex);
        files.swap(m_changedFiles);
    }
    
    if (!files.empty())
    {
        LT_LOGI("AssetManager", std::format("Processing {} file change(s)...", files.size()));
    }

    for (const auto& f : files)
    {
        LT_ASSERT_MSG(!f.empty(), "Changed file path is empty");
        
        std::string ext = std::filesystem::path(f).extension().string();
        if (auto* importer = m_importers.findImporter(ext))
        {
            LT_ASSERT_MSG(importer, "Found importer is null");
            LT_LOGI("AssetManager", "Reimport: " + f);

            std::filesystem::path full = f;
            LT_ASSERT_MSG(std::filesystem::exists(full), "Changed file does not exist: " + f);
            
            std::filesystem::path rel;

            if (full.string().starts_with(m_projectResourcesRoot.string()))
                rel = std::filesystem::relative(full, m_projectResourcesRoot);
            else if (full.string().starts_with(m_engineResourcesRoot.string()))
                rel = std::filesystem::relative(full, m_engineResourcesRoot);
            else
                rel = full;

            auto info = importer->import(full, m_cacheRoot);
            LT_ASSERT_MSG(!info.guid.empty(), "Imported asset has empty GUID");
            LT_ASSERT_MSG(!info.sourcePath.empty(), "Imported asset has empty source path");
            
            info.sourcePath = rel.generic_string();
            info.guid       = MakeDeterministicIDFromPath(info.sourcePath);
            
            LT_ASSERT_MSG(!info.guid.empty(), "Generated GUID is empty");

            m_database.upsert(info);
            LT_LOGI("AssetManager", std::format("Reimported [{}] {}", info.guid.str(), info.sourcePath));
        }
        else
        {
            LT_LOGW("AssetManager", "No importer for extension: " + ext);
        }
    }
    
    if (!files.empty())
    {
        LT_LOGI("AssetManager", std::format("Processed {} file change(s)", files.size()));
    }
}

// --------------------------------------------------------

void AssetManager::scanAndImportAllIn(const std::filesystem::path& root)
{
    ZoneScopedN("AssetManager::scanAndImportAllIn");
    LT_ASSERT_MSG(!root.empty(), "Scan root path cannot be empty");
    
    if (!std::filesystem::exists(root))
    {
        LT_LOGE("AssetManager", "Directory does not exist: " + root.string());
        return;
    }
    
    LT_ASSERT_MSG(std::filesystem::is_directory(root), "Root path is not a directory: " + root.string());
    
    LT_LOGI("AssetManager", std::format("Scanning directory: {}", root.string()));

    size_t filesFound = 0;
    size_t filesImported = 0;
    size_t filesSkipped = 0;
    for (auto& entry : std::filesystem::recursive_directory_iterator(root))
    {
        if (!entry.is_regular_file())
            continue;
            
        auto ext = entry.path().extension().string();
        auto importer = m_importers.findImporter(ext);
        if (!importer)
            continue;
        
        LT_ASSERT_MSG(importer, "Found importer is null");
        filesFound++;
        
        // --- Добавляем нормализацию пути ---
        std::filesystem::path abs = std::filesystem::weakly_canonical(entry.path());
        LT_ASSERT_MSG(std::filesystem::exists(abs), "Canonical path does not exist");
        
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

        // --- Вычисляем ожидаемый GUID ДО импорта ---
        AssetID expectedGuid = MakeDeterministicIDFromPath(rel.generic_string());
        LT_ASSERT_MSG(!expectedGuid.empty(), "Generated GUID is empty");
        
        // --- Проверяем, существует ли ассет в базе ---
        auto existingInfoOpt = m_database.get(expectedGuid);
        
        // Получаем timestamp и размер файла для сравнения
        auto fileTime = std::filesystem::last_write_time(abs);
        auto fileTimeCount = fileTime.time_since_epoch().count();
        uint64_t currentFileSize = 0;
        try
        {
            if (std::filesystem::exists(abs))
            {
                currentFileSize = static_cast<uint64_t>(std::filesystem::file_size(abs));
            }
        }
        catch (...)
        {
            // Игнорируем ошибки получения размера файла
        }
        
        // Если ассет уже существует в базе и файл не изменился, пропускаем импорт
        if (existingInfoOpt)
        {
            const auto& existing = *existingInfoOpt;
            bool fileUnchanged = (existing.sourceTimestamp == static_cast<uint64_t>(fileTimeCount)) &&
                                 (existing.sourceFileSize == currentFileSize);
            
            if (fileUnchanged)
            {
                LT_LOGI("AssetManager", std::format("Skipping unchanged asset [{}] {}", 
                    expectedGuid.str(), rel.generic_string()));
                filesSkipped++;
                continue;
            }
            
            LT_LOGI("AssetManager", std::format("Reimporting changed asset [{}] {} (timestamp: {} -> {})", 
                expectedGuid.str(), rel.generic_string(), 
                existing.sourceTimestamp, static_cast<uint64_t>(fileTimeCount)));
        }

        // --- Импорт (только если нужно) ---
        LT_ASSERT_MSG(!m_cacheRoot.empty(), "Cache root is not set");
        auto info = importer->import(abs, m_cacheRoot);
        
        // После импорта GUID должен быть валидным (импортировали реальный файл)
        LT_ASSERT_MSG(!info.guid.empty(), "Imported asset has empty GUID");

        // --- Перезаписываем корректные поля ---
        info.sourcePath = rel.generic_string();
        info.guid       = expectedGuid;  // Используем предвычисленный GUID для консистентности
        info.origin     = origin;
        
        // Убеждаемся, что timestamp установлен (если импортер его не установил)
        if (info.sourceTimestamp == 0)
        {
            info.sourceTimestamp = static_cast<uint64_t>(fileTimeCount);
        }
        if (info.sourceFileSize == 0 && currentFileSize > 0)
        {
            info.sourceFileSize = currentFileSize;
        }
        
        // После генерации из пути GUID должен быть валидным
        LT_ASSERT_MSG(!info.guid.empty(), "Generated GUID is empty after path determinization");
        LT_ASSERT_MSG(!info.sourcePath.empty(), "Source path is empty");

        m_database.upsert(info);
        filesImported++;

        LT_LOGI("AssetManager", std::format("Imported [{}] {}", info.guid.str(), info.sourcePath));
    }
    
    LT_LOGI("AssetManager", std::format("Scan completed: {} file(s) found, {} imported, {} skipped", 
        filesFound, filesImported, filesSkipped));
}

// --------------------------------------------------------

void AssetManager::scanAndImportAll()
{
    ZoneScopedN("AssetManager::scanAndImportAll");
    LT_ASSERT_MSG(!m_engineResourcesRoot.empty(), "Engine resources root not set");
    LT_ASSERT_MSG(!m_projectResourcesRoot.empty(), "Project resources root not set");
    
    scanAndImportAllIn(m_engineResourcesRoot);
    scanAndImportAllIn(m_projectResourcesRoot);
}

// --------------------------------------------------------

void AssetManager::saveDatabase()
{
    ZoneScopedN("AssetManager::saveDatabase");
    LT_ASSERT_MSG(!m_dbPath.empty(), "Database path is empty");
    
    LT_LOGI("AssetManager", "Saving asset database to: " + m_dbPath.string());
    bool saved = m_database.save(m_dbPath.string());
    if (!saved)
    {
        LT_LOGE("AssetManager", "Failed to save database to: " + m_dbPath.string());
    }
    else
    {
        LT_LOGI("AssetManager", "Asset database saved successfully");
    }
}
