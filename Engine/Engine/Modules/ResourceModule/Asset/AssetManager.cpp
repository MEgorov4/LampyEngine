#include "AssetManager.h"
#include <Foundation/Assert/Assert.h>
#include <Core/CoreGlobal.h>
#include <Foundation/JobSystem/JobSystem.h>
#include "Foundation/Memory/MemoryMacros.h"
#include "Foundation/Memory/ResourceAllocator.h"

#include "Importers/MeshImporter.h"
#include "Importers/ShaderImporter.h"
#include "Importers/TextureImporter.h"
#include "Importers/WorldImporter.h"
#include "Importers/MaterialImporter.h"
#include "Importers/ScriptImporter.h"

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

    m_watcher  = std::make_unique<efsw::FileWatcher>();
    LT_ASSERT_MSG(m_watcher, "Failed to create FileWatcher");
    
    m_listener = std::make_unique<AssetFileListener>(this);
    LT_ASSERT_MSG(m_listener, "Failed to create AssetFileListener");

    registerDefaultImporters();

    LT_ASSERT_MSG(std::filesystem::exists(m_engineResourcesRoot), "Engine resources root does not exist");
    LT_ASSERT_MSG(std::filesystem::exists(m_projectResourcesRoot), "Project resources root does not exist");
    
    LT_LOGI("AssetManager", "Starting file watchers...");
    watchDirectory(m_engineResourcesRoot.string());
    watchDirectory(m_projectResourcesRoot.string());
    LT_LOGI("AssetManager", "File watchers started");

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

    // Stop file watcher before joining thread
    if (m_watcher)
    {
        // Remove all watches to allow watch() to exit
        // Note: efsw doesn't have explicit stop method, but removing watches should help
        m_watcher.reset();
    }

    // Wait for watch thread to finish
    if (m_watchThread && m_watchThread->joinable())
    {
        // Give the thread a moment to exit after watcher is destroyed
        // If it doesn't exit, we'll timeout and detach (not ideal but prevents hang)
        m_watchThread->join();
    }
    
    m_listener.reset();
}

// --------------------------------------------------------

namespace
{
    using namespace EngineCore::Foundation;
    using namespace ResourceModule;
    
    // Helper deleter for TextureImporter
    void deleteTextureImporter(IAssetImporter* p)
    {
        if (p)
        {
            static_cast<TextureImporter*>(p)->~TextureImporter();
            DeallocateMemory(p, MemoryTag::Resource);
        }
    }
    
    // Helper deleter for ShaderImporter
    void deleteShaderImporter(IAssetImporter* p)
    {
        if (p)
        {
            static_cast<ShaderImporter*>(p)->~ShaderImporter();
            DeallocateMemory(p, MemoryTag::Resource);
        }
    }
    
    // Helper deleter for MeshImporter
    void deleteMeshImporter(IAssetImporter* p)
    {
        if (p)
        {
            static_cast<MeshImporter*>(p)->~MeshImporter();
            DeallocateMemory(p, MemoryTag::Resource);
        }
    }
    
    // Helper deleter for WorldImporter
    void deleteWorldImporter(IAssetImporter* p)
    {
        if (p)
        {
            static_cast<WorldImporter*>(p)->~WorldImporter();
            DeallocateMemory(p, MemoryTag::Resource);
        }
    }
    
    // Helper deleter for MaterialImporter
    void deleteMaterialImporter(IAssetImporter* p)
    {
        if (p)
        {
            static_cast<MaterialImporter*>(p)->~MaterialImporter();
            DeallocateMemory(p, MemoryTag::Resource);
        }
    }
    
    // Helper deleter for ScriptImporter
    void deleteScriptImporter(IAssetImporter* p)
    {
        if (p)
        {
            static_cast<ScriptImporter*>(p)->~ScriptImporter();
            DeallocateMemory(p, MemoryTag::Resource);
        }
    }
}

void AssetManager::registerDefaultImporters()
{
    ZoneScopedN("AssetManager::registerDefaultImporters");
    LT_LOGI("AssetManager", "Registering default asset importers...");
    
    using namespace EngineCore::Foundation;
    
    // Allocate and create TextureImporter using MemorySystem
    void* textureImporterMemory = AllocateMemory(sizeof(TextureImporter), alignof(TextureImporter), MemoryTag::Resource);
    if (textureImporterMemory)
    {
        TextureImporter* textureImporter = nullptr;
        try
        {
            textureImporter = new(textureImporterMemory) TextureImporter();
            using DeleterType = void(*)(IAssetImporter*);
            DeleterType deleter = deleteTextureImporter;
            std::unique_ptr<IAssetImporter, DeleterType> importer(textureImporter, deleter);
            m_importers.registerImporter(std::move(importer));
            LT_LOGI("AssetManager", "Registered TextureImporter");
        }
        catch (...)
        {
            DeallocateMemory(textureImporterMemory, MemoryTag::Resource);
            LT_LOGE("AssetManager", "Failed to create TextureImporter: constructor threw exception");
        }
    }
    else
    {
        LT_LOGE("AssetManager", "Failed to allocate memory for TextureImporter");
    }
    
    // Allocate and create ShaderImporter using MemorySystem
    void* shaderImporterMemory = AllocateMemory(sizeof(ShaderImporter), alignof(ShaderImporter), MemoryTag::Resource);
    if (shaderImporterMemory)
    {
        ShaderImporter* shaderImporter = nullptr;
        try
        {
            shaderImporter = new(shaderImporterMemory) ShaderImporter();
            using DeleterType = void(*)(IAssetImporter*);
            DeleterType deleter = deleteShaderImporter;
            std::unique_ptr<IAssetImporter, DeleterType> importer(shaderImporter, deleter);
            m_importers.registerImporter(std::move(importer));
            LT_LOGI("AssetManager", "Registered ShaderImporter");
        }
        catch (...)
        {
            DeallocateMemory(shaderImporterMemory, MemoryTag::Resource);
            LT_LOGE("AssetManager", "Failed to create ShaderImporter: constructor threw exception");
        }
    }
    else
    {
        LT_LOGE("AssetManager", "Failed to allocate memory for ShaderImporter");
    }
    
    // Allocate and create MeshImporter using MemorySystem
    void* meshImporterMemory = AllocateMemory(sizeof(MeshImporter), alignof(MeshImporter), MemoryTag::Resource);
    if (meshImporterMemory)
    {
        MeshImporter* meshImporter = nullptr;
        try
        {
            meshImporter = new(meshImporterMemory) MeshImporter();
            using DeleterType = void(*)(IAssetImporter*);
            DeleterType deleter = deleteMeshImporter;
            std::unique_ptr<IAssetImporter, DeleterType> importer(meshImporter, deleter);
            m_importers.registerImporter(std::move(importer));
            LT_LOGI("AssetManager", "Registered MeshImporter");
        }
        catch (...)
        {
            DeallocateMemory(meshImporterMemory, MemoryTag::Resource);
            LT_LOGE("AssetManager", "Failed to create MeshImporter: constructor threw exception");
        }
    }
    else
    {
        LT_LOGE("AssetManager", "Failed to allocate memory for MeshImporter");
    }
    
    // Allocate and create WorldImporter using MemorySystem
    void* worldImporterMemory = AllocateMemory(sizeof(WorldImporter), alignof(WorldImporter), MemoryTag::Resource);
    if (worldImporterMemory)
    {
        WorldImporter* worldImporter = nullptr;
        try
        {
            worldImporter = new(worldImporterMemory) WorldImporter();
            using DeleterType = void(*)(IAssetImporter*);
            DeleterType deleter = deleteWorldImporter;
            std::unique_ptr<IAssetImporter, DeleterType> importer(worldImporter, deleter);
            m_importers.registerImporter(std::move(importer));
            LT_LOGI("AssetManager", "Registered WorldImporter");
        }
        catch (...)
        {
            DeallocateMemory(worldImporterMemory, MemoryTag::Resource);
            LT_LOGE("AssetManager", "Failed to create WorldImporter: constructor threw exception");
        }
    }
    else
    {
        LT_LOGE("AssetManager", "Failed to allocate memory for WorldImporter");
    }
    
    // Allocate and create MaterialImporter using MemorySystem
    void* materialImporterMemory = AllocateMemory(sizeof(MaterialImporter), alignof(MaterialImporter), MemoryTag::Resource);
    if (materialImporterMemory)
    {
        MaterialImporter* materialImporter = nullptr;
        try
        {
            materialImporter = new(materialImporterMemory) MaterialImporter();
            using DeleterType = void(*)(IAssetImporter*);
            DeleterType deleter = deleteMaterialImporter;
            std::unique_ptr<IAssetImporter, DeleterType> importer(materialImporter, deleter);
            m_importers.registerImporter(std::move(importer));
            LT_LOGI("AssetManager", "Registered MaterialImporter");
        }
        catch (...)
        {
            DeallocateMemory(materialImporterMemory, MemoryTag::Resource);
            LT_LOGE("AssetManager", "Failed to create MaterialImporter: constructor threw exception");
        }
    }
    else
    {
        LT_LOGE("AssetManager", "Failed to allocate memory for MaterialImporter");
    }
    
    // Allocate and create ScriptImporter using MemorySystem
    void* scriptImporterMemory = AllocateMemory(sizeof(ScriptImporter), alignof(ScriptImporter), MemoryTag::Resource);
    if (scriptImporterMemory)
    {
        ScriptImporter* scriptImporter = nullptr;
        try
        {
            scriptImporter = new(scriptImporterMemory) ScriptImporter();
            using DeleterType = void(*)(IAssetImporter*);
            DeleterType deleter = deleteScriptImporter;
            std::unique_ptr<IAssetImporter, DeleterType> importer(scriptImporter, deleter);
            m_importers.registerImporter(std::move(importer));
            LT_LOGI("AssetManager", "Registered ScriptImporter");
        }
        catch (...)
        {
            DeallocateMemory(scriptImporterMemory, MemoryTag::Resource);
            LT_LOGE("AssetManager", "Failed to create ScriptImporter: constructor threw exception");
        }
    }
    else
    {
        LT_LOGE("AssetManager", "Failed to allocate memory for ScriptImporter");
    }
    
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
    std::vector<std::string, ResourceAllocator<std::string>> files;
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
            OnAssetImported(info);
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

        AssetID expectedGuid = MakeDeterministicIDFromPath(rel.generic_string());
        LT_ASSERT_MSG(!expectedGuid.empty(), "Generated GUID is empty");
        
        auto existingInfoOpt = m_database.get(expectedGuid);
        
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
        }
        
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

        LT_ASSERT_MSG(!m_cacheRoot.empty(), "Cache root is not set");
        auto info = importer->import(abs, m_cacheRoot);
        
        LT_ASSERT_MSG(!info.guid.empty(), "Imported asset has empty GUID");

        info.sourcePath = rel.generic_string();
        info.guid       = expectedGuid;
        info.origin     = origin;
        
        if (info.sourceTimestamp == 0)
        {
            info.sourceTimestamp = static_cast<uint64_t>(fileTimeCount);
        }
        if (info.sourceFileSize == 0 && currentFileSize > 0)
        {
            info.sourceFileSize = currentFileSize;
        }
        
        LT_ASSERT_MSG(!info.guid.empty(), "Generated GUID is empty after path determinization");
        LT_ASSERT_MSG(!info.sourcePath.empty(), "Source path is empty");

        m_database.upsert(info);
        OnAssetImported(info);
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

// --------------------------------------------------------

EngineCore::Foundation::JobHandle AssetManager::scheduleRescanJob()
{
    using namespace EngineCore::Foundation;
    
    auto* jobSystem = GCM(JobSystem);
    if (!jobSystem)
    {
        LT_LOGW("AssetManager", "JobSystem not available, running rescan synchronously");
        scanAndImportAll();
        return JobHandle();
    }
    
    LT_LOGI("AssetManager", "Scheduling rescan job");
    return jobSystem->submit([this]() {
        LT_LOGI("AssetManager", "Running rescan job");
        scanAndImportAll();
        saveDatabase();
        LT_LOGI("AssetManager", "Rescan job completed");
    }, "AssetManager::Rescan");
}
