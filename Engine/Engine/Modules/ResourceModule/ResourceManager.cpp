#include "ResourceManager.h"
#include "Pak/PakReader.h"
#include "Foundation/Assert/Assert.h"
#include "Foundation/Memory/MemoryMacros.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Script.h"
#include "RWorld.h"
#include "../../Core/CoreGlobal.h"
#include "../../Foundation/JobSystem/JobSystem.h"
#include "../TimeModule/TimeModule.h"
#include <filesystem>
#include <fstream>

using namespace ResourceModule;

void ResourceManager::startup()
{
    LT_LOGI("ResourceManager", "Startup");
    
    // Start periodic cleanup of expired cache entries
    // Cleanup runs every 1 second in a background job
    // Use tryGet() to handle case when TimeModule is not available (e.g., in tests)
    auto timeModule = Core::Locator().tryGet<TimeModule::TimeModule>();
    if (timeModule)
    {
        m_periodicCleanupEnabled = true;
        m_cleanupTaskId = timeModule->getScheduler().scheduleRepeating([this]() {
            if (m_periodicCleanupEnabled)
            {
                scheduleCleanupExpiredJob();
            }
        }, 1.0); // Every 1 second
        LT_LOGI("ResourceManager", "Periodic cache cleanup enabled (every 1 second)");
    }
    else
    {
        LT_LOGW("ResourceManager", "TimeModule not available, periodic cleanup disabled");
    }
}

void ResourceManager::shutdown()
{
    LT_LOGI("ResourceManager", "Shutdown");
    
    // Disable periodic cleanup (the scheduled task will check the flag and skip execution)
    m_periodicCleanupEnabled = false;

    if (m_cleanupTaskId != TimeModule::TimeScheduler::InvalidTaskId)
    {
        if (auto timeModule = Core::Locator().tryGet<TimeModule::TimeModule>())
        {
            timeModule->getScheduler().cancel(m_cleanupTaskId);
        }
        m_cleanupTaskId = TimeModule::TimeScheduler::InvalidTaskId;
    }
    
    clearAll();
}

void ResourceManager::mountPak(const std::string &pakPath)
{
    LT_ASSERT_MSG(!pakPath.empty(), "PAK path cannot be empty");
    
    using namespace EngineCore::Foundation;
    // Allocate memory for PakReader using MemorySystem
    void* pakReaderMemory = AllocateMemory(sizeof(PakReader), alignof(PakReader), MemoryTag::Resource);
    if (!pakReaderMemory)
    {
        LT_LOGE("ResourceManager", "Failed to allocate memory for PakReader");
        return;
    }
    
    // Construct PakReader using placement new
    PakReader* pakReaderPtr = nullptr;
    try
    {
        pakReaderPtr = new(pakReaderMemory) PakReader(pakPath);
    }
    catch (...)
    {
        DeallocateMemory(pakReaderMemory, MemoryTag::Resource);
        LT_LOGE("ResourceManager", "Failed to construct PakReader: constructor threw exception");
        return;
    }
    
    // Create deleter using std::function to allow lambda capture
    std::function<void(PakReader*)> deleter = [](PakReader* p) {
        using namespace EngineCore::Foundation;
        if (p)
        {
            p->~PakReader();
            DeallocateMemory(p, MemoryTag::Resource);
        }
    };
    
    // Create unique_ptr with custom deleter
    m_pakReader = std::unique_ptr<PakReader, std::function<void(PakReader*)>>(pakReaderPtr, deleter);
    
    LT_ASSERT_MSG(m_pakReader, "Failed to create PakReader");
    
    m_usePak = m_pakReader->isOpen();
    if (m_usePak)
        LT_LOGI("ResourceManager", "Mounted PAK: " + pakPath);
    else
        LT_LOGW("ResourceManager", "Failed to mount PAK: " + pakPath);
}

void ResourceManager::unload(const AssetID &guid)
{
    if (guid.empty())
        return;
    
    m_registry.unregister(guid);
    
    getCache<RMaterial>().remove(guid);
    getCache<RMesh>().remove(guid);
    getCache<RTexture>().remove(guid);
    getCache<RShader>().remove(guid);
    getCache<RScript>().remove(guid);
    getCache<RWorld>().remove(guid);
    
    LT_LOGI("ResourceManager", std::format("Unloaded resource [{}]", guid.str()));
}

void ResourceManager::clearAll()
{
    LT_LOGI("ResourceManager", "Clearing all resources");
    
    // CRITICAL: Clear registry FIRST to release all shared_ptr references
    // ResourceRegistry holds shared_ptr, which keeps resources alive
    // This must be done before clearing caches to allow memory to be freed
    m_registry.clear();
    
    // Remove unused entries from caches (weak_ptr that are no longer valid)
    cleanupExpiredCacheEntries();
    
    // Then clear all caches completely
    // Note: clear() only removes weak_ptr entries from the map
    // Actual memory is freed when the last shared_ptr to a resource is destroyed
    // By clearing registry first, we ensure resources can be freed
    getCache<RMaterial>().clear();
    getCache<RMesh>().clear();
    getCache<RTexture>().clear();
    getCache<RShader>().clear();
    getCache<RScript>().clear();
    getCache<RWorld>().clear();
    
    // Final cleanup pass to remove any remaining expired entries
    cleanupExpiredCacheEntries();
    
    LT_LOGI("ResourceManager", "All resources cleared");
}

void ResourceManager::cleanupExpiredCacheEntries()
{
    getCache<RMaterial>().removeUnused();
    getCache<RMesh>().removeUnused();
    getCache<RTexture>().removeUnused();
    getCache<RShader>().removeUnused();
    getCache<RScript>().removeUnused();
    getCache<RWorld>().removeUnused();
}

EngineCore::Foundation::JobHandle ResourceManager::scheduleCleanupExpiredJob()
{
    using namespace EngineCore::Foundation;

    cleanupExpiredCacheEntries();
    return JobHandle();
}

WriterContext ResourceManager::buildWriterContext()
{
    WriterContext ctx;
    ctx.database             = m_assetDatabase;
    ctx.registry             = &m_registry;
    ctx.engineResourcesRoot  = m_engineResourcesRoot;
    ctx.projectResourcesRoot = m_projectResourcesRoot;
    ctx.cacheRoot            = m_cacheRoot;
    ctx.usePak               = m_usePak;
    return ctx;
}

std::shared_ptr<BaseResource> ResourceManager::findLoadedResource(const AssetID &id) const
{
    return m_registry.get(id);
}

std::filesystem::path ResourceManager::resolveTargetPath(const AssetInfo &info,
                                                         const ResourceSaveParams &params) const
{
    if (!params.targetPathOverride.empty())
        return params.targetPathOverride;
    if (!info.importedPath.empty())
        return std::filesystem::path(info.importedPath);
    return {};
}

std::filesystem::path ResourceManager::resolveSourcePath(const AssetInfo &info,
                                                         const ResourceSaveParams &params) const
{
    if (!params.sourcePathOverride.empty())
        return params.sourcePathOverride;
    if (!info.sourcePath.empty())
        return std::filesystem::path(info.sourcePath);
    return {};
}

std::string ResourceManager::relativeToKnownRoots(const std::filesystem::path &path, AssetOrigin &origin) const
{
    std::error_code ec;
    if (!m_projectResourcesRoot.empty())
    {
        auto rel = std::filesystem::relative(path, m_projectResourcesRoot, ec);
        if (!ec)
        {
            origin = AssetOrigin::Project;
            return rel.generic_string();
        }
    }

    ec.clear();
    if (!m_engineResourcesRoot.empty())
    {
        auto rel = std::filesystem::relative(path, m_engineResourcesRoot, ec);
        if (!ec)
        {
            origin = AssetOrigin::Engine;
            return rel.generic_string();
        }
    }

    origin = AssetOrigin::Project;
    return path.generic_string();
}

AssetInfo ResourceManager::composeAssetInfo(AssetInfo base, const std::filesystem::path &targetPath,
                                            const std::filesystem::path &sourcePathOverride) const
{
    std::filesystem::path normalizedTarget = targetPath;
    std::error_code ec;
    normalizedTarget = std::filesystem::weakly_canonical(normalizedTarget, ec);
    if (ec)
        normalizedTarget = std::filesystem::path(targetPath);

    if (!sourcePathOverride.empty())
    {
        base.sourcePath = sourcePathOverride.generic_string();
    }
    else if (base.sourcePath.empty())
    {
        AssetOrigin inferredOrigin = base.origin;
        base.sourcePath            = relativeToKnownRoots(normalizedTarget, inferredOrigin);
        base.origin                = inferredOrigin;
    }

    base.importedPath = normalizedTarget.generic_string();

    ec.clear();
    if (std::filesystem::exists(normalizedTarget, ec) && !ec)
    {
        std::error_code sizeEc;
        auto size = std::filesystem::file_size(normalizedTarget, sizeEc);
        if (!sizeEc)
            base.importedFileSize = static_cast<uint64_t>(size);

        std::error_code timeEc;
        auto ts = std::filesystem::last_write_time(normalizedTarget, timeEc);
        if (!timeEc)
            base.importedTimestamp = static_cast<uint64_t>(ts.time_since_epoch().count());
    }

    return base;
}

bool ResourceManager::updateDatabaseEntry(const AssetInfo &existing, const std::filesystem::path &targetPath,
                                          const std::filesystem::path &sourcePathOverride)
{
    if (!m_assetDatabase)
    {
        LT_LOGE("ResourceManager", "AssetDatabase not available while updating entry");
        return false;
    }

    AssetInfo updated = composeAssetInfo(existing, targetPath, sourcePathOverride);
    m_assetDatabase->upsert(updated);
    return true;
}

bool ResourceManager::save(const AssetID &id, const ResourceSaveParams &params)
{
    if (id.empty())
        return false;
    if (!m_assetDatabase)
    {
        LT_LOGE("ResourceManager", "Cannot save resource without AssetDatabase");
        return false;
    }
    if (!m_writerHub)
    {
        LT_LOGE("ResourceManager", "Cannot save resource without AssetWriterHub");
        return false;
    }

    auto infoOpt = m_assetDatabase->get(id);
    if (!infoOpt)
    {
        LT_LOGW("ResourceManager", "AssetInfo not found for GUID: " + id.str());
        return false;
    }

    const auto &info = *infoOpt;
    auto writer      = m_writerHub->findWriter(info.type);
    if (!writer)
    {
        LT_LOGW("ResourceManager", "No writer registered for asset type");
        return false;
    }

    auto resource = findLoadedResource(id);
    if (!resource)
    {
        LT_LOGW("ResourceManager", "Resource not loaded, cannot save: " + id.str());
        return false;
    }

    auto targetPath = resolveTargetPath(info, params);
    if (targetPath.empty())
    {
        LT_LOGE("ResourceManager", "Target path is empty for save operation");
        return false;
    }

    if (params.createDirectories)
    {
        std::error_code dirEc;
        auto parent = targetPath.parent_path();
        if (!parent.empty())
        {
            std::filesystem::create_directories(parent, dirEc);
            if (dirEc)
            {
                LT_LOGE("ResourceManager", "Failed to create directories for save: " + dirEc.message());
                return false;
            }
        }
    }

    auto ctx = buildWriterContext();
    if (!ctx.isValid())
    {
        LT_LOGE("ResourceManager", "WriterContext is invalid (missing database)");
        return false;
    }

    if (!writer->write(*resource, targetPath, ctx))
        return false;

    if (params.updateDatabase)
    {
        auto sourceOverride = resolveSourcePath(info, params);
        updateDatabaseEntry(info, targetPath, sourceOverride);
    }

    return true;
}

std::optional<AssetID> ResourceManager::saveAs(const AssetID &id, const std::filesystem::path &targetPath,
                                               const ResourceSaveParams &params)
{
    if (id.empty())
        return std::nullopt;
    if (!m_assetDatabase)
    {
        LT_LOGE("ResourceManager", "Cannot save-as without AssetDatabase");
        return std::nullopt;
    }

    auto infoOpt = m_assetDatabase->get(id);
    if (!infoOpt)
    {
        LT_LOGW("ResourceManager", "AssetInfo not found for save-as GUID: " + id.str());
        return std::nullopt;
    }

    auto resource = findLoadedResource(id);
    if (!resource)
    {
        LT_LOGW("ResourceManager", "Resource not loaded for save-as: " + id.str());
        return std::nullopt;
    }

    ResourceSaveParams derivedParams = params;
    if (!derivedParams.originOverride)
        derivedParams.originOverride = infoOpt->origin;

    if (derivedParams.sourcePathOverride.empty())
    {
        derivedParams.sourcePathOverride = targetPath;
    }

    return saveAs(*resource, infoOpt->type, targetPath, derivedParams);
}

std::optional<AssetID> ResourceManager::saveAs(const BaseResource &resource, AssetType type,
                                               const std::filesystem::path &targetPath,
                                               const ResourceSaveParams &params)
{
    if (!m_writerHub)
    {
        LT_LOGE("ResourceManager", "Cannot save-as without AssetWriterHub");
        return std::nullopt;
    }

    auto writer = m_writerHub->findWriter(type);
    if (!writer)
    {
        LT_LOGW("ResourceManager", "No writer registered for asset type");
        return std::nullopt;
    }

    if (params.createDirectories)
    {
        std::error_code dirEc;
        auto parent = targetPath.parent_path();
        if (!parent.empty())
        {
            std::filesystem::create_directories(parent, dirEc);
            if (dirEc)
            {
                LT_LOGE("ResourceManager", "Failed to create directories for save-as: " + dirEc.message());
                return std::nullopt;
            }
        }
    }

    auto ctx = buildWriterContext();
    if (!ctx.isValid())
    {
        LT_LOGE("ResourceManager", "WriterContext is invalid (missing database)");
        return std::nullopt;
    }

    if (!writer->write(resource, targetPath, ctx))
        return std::nullopt;

    AssetID newGuid = params.explicitGuid ? *params.explicitGuid : MakeRandomAssetID();
    AssetInfo newInfo;
    newInfo.guid  = newGuid;
    newInfo.type  = type;
    newInfo.origin = params.originOverride.value_or(AssetOrigin::Project);

    std::filesystem::path resolvedSource =
        params.sourcePathOverride.empty() ? targetPath : params.sourcePathOverride;

    std::string sourceString;
    if (params.sourcePathOverride.empty())
    {
        auto origin = newInfo.origin;
        sourceString = relativeToKnownRoots(resolvedSource, origin);
        newInfo.origin = origin;
    }
    else
    {
        sourceString = params.sourcePathOverride.generic_string();
    }
    newInfo.sourcePath = sourceString;

    if (params.updateDatabase && m_assetDatabase)
    {
        std::filesystem::path overridePath =
            params.sourcePathOverride.empty() ? std::filesystem::path{} : std::filesystem::path(sourceString);
        AssetInfo updated = composeAssetInfo(newInfo, targetPath, overridePath);
        m_assetDatabase->upsert(updated);
    }

    return newGuid;
}
