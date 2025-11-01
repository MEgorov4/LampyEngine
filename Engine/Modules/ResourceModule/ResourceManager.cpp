#include "ResourceManager.h"
#include "Pak/PakReader.h"
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
    m_pakReader = std::make_unique<PakReader>(pakPath);
    m_usePak = m_pakReader->isOpen();
    if (m_usePak)
        LT_LOGI("ResourceManager", "Mounted PAK: " + pakPath);
    else
        LT_LOGW("ResourceManager", "Failed to mount PAK: " + pakPath);
}

void ResourceManager::unload(const AssetID &guid)
{
    m_registry.unregister(guid);
}

void ResourceManager::clearAll()
{
    m_registry.clear();
}
