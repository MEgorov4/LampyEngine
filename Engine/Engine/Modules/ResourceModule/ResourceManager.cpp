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
        timeModule->getScheduler().scheduleRepeating([this]() {
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
    
    auto* jobSystem = GCM(JobSystem);
    if (!jobSystem)
    {
        // Fallback to synchronous execution if JobSystem is not available
        cleanupExpiredCacheEntries();
        return JobHandle();
    }
    
    // Schedule cleanup job in background thread
    // No logging here as this is called frequently (every second)
    return jobSystem->submit([this]() {
        cleanupExpiredCacheEntries();
    }, "ResourceManager::CleanupExpiredCache");
}
