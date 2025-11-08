#include "ResourceManager.h"
#include "Pak/PakReader.h"
#include "Foundation/Assert/Assert.h"
#include <filesystem>
#include <fstream>

using namespace ResourceModule;

void ResourceManager::startup()
{
    LT_LOGI("ResourceManager", "Startup");
}

void ResourceManager::shutdown()
{
    LT_LOGI("ResourceManager", "Shutdown");
    clearAll();
}

void ResourceManager::mountPak(const std::string &pakPath)
{
    LT_ASSERT_MSG(!pakPath.empty(), "PAK path cannot be empty");
    
    m_pakReader = std::make_unique<PakReader>(pakPath);
    LT_ASSERT_MSG(m_pakReader, "Failed to create PakReader");
    
    m_usePak = m_pakReader->isOpen();
    if (m_usePak)
        LT_LOGI("ResourceManager", "Mounted PAK: " + pakPath);
    else
        LT_LOGW("ResourceManager", "Failed to mount PAK: " + pakPath);
}

void ResourceManager::unload(const AssetID &guid)
{
    // Пустой AssetID допустим - просто ничего не делаем
    if (guid.empty())
        return;
    
    LT_LOGI("ResourceManager", "Unloading resource: " + guid.str());
    m_registry.unregister(guid);
}

void ResourceManager::clearAll()
{
    LT_LOGI("ResourceManager", "Clearing all resources");
    m_registry.clear();
}
