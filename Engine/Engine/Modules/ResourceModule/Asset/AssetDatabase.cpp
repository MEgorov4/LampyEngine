#include "AssetDatabase.h"
#include <Foundation/Assert/Assert.h>

#include <filesystem>
#include <fstream>

using namespace ResourceModule;
using EngineCore::Foundation::ResourceAllocator;

static std::string NormalizePath(std::string path)
{
#ifdef _WIN32
    for (auto& c : path)
    {
        if (c == '/')
            c = '\\';
        else
            c = (char) tolower(c);
    }
#else
    for (auto& c : path)
        if (c == '\\')
            c = '/';
#endif
    return path;
}

std::vector<AssetInfo, ResourceAllocator<AssetInfo>> AssetDatabase::getByOrigin(AssetOrigin origin) const
{
    std::shared_lock lock(m_mutex);
    std::vector<AssetInfo, ResourceAllocator<AssetInfo>> result;
    result.reserve(m_assets.size());
    for (const auto& [_, info] : m_assets)
        if (info.origin == origin)
            result.push_back(info);
    return result;
}

bool AssetDatabase::load(const std::string& path)
{
    LT_ASSERT_MSG(!path.empty(), "Database load path cannot be empty");
    
    std::ifstream ifs(path);
    if (!ifs.is_open())
        return false;

    nlohmann::json j;
    try
    {
        ifs >> j;
    }
    catch (const nlohmann::json::exception& e)
    {
        LT_LOGE("AssetDatabase", "Failed to parse JSON: " + std::string(e.what()));
        return false;
    }

    std::unique_lock lock(m_mutex);
    m_assets.clear();
    m_sourceToGuid.clear();

    for (auto& [guidStr, infoJson] : j.items())
    {
        AssetInfo info;
        try
        {
            info = infoJson.get<AssetInfo>();
        }
        catch (const nlohmann::json::exception& e)
        {
            LT_LOGE("AssetDatabase", "Failed to parse AssetInfo from JSON: " + std::string(e.what()));
            continue;
        }
        
        if (info.guid.empty() || info.sourcePath.empty())
        {
            LT_LOGW("AssetDatabase", "Skipping invalid asset entry with empty GUID or source path");
            continue;
        }
        
        m_sourceToGuid[NormalizePath(info.sourcePath)] = info.guid;
        m_assets.emplace(info.guid, std::move(info));
    }
    return true;
}

bool AssetDatabase::save(const std::string& path) const
{
    LT_ASSERT_MSG(!path.empty(), "Database save path cannot be empty");
    
    nlohmann::json j;
    {
        std::shared_lock lock(m_mutex);
        for (const auto& [guid, info] : m_assets)
        {
            if (guid.empty() || info.sourcePath.empty())
            {
                LT_LOGW("AssetDatabase", "Skipping invalid asset with empty GUID or source path when saving");
                continue;
            }
            j[guid.str()] = info;
        }
    }

    std::filesystem::path dbPath(path);
    std::filesystem::create_directories(dbPath.parent_path());

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        LT_LOGE("AssetDatabase", "Failed to open file for writing: " + path);
        return false;
    }
    
    ofs << j.dump(2);
    return true;
}

void AssetDatabase::upsert(const AssetInfo& info)
{
    LT_ASSERT_MSG(!info.guid.empty(), "Cannot upsert asset with empty GUID");
    LT_ASSERT_MSG(!info.sourcePath.empty(), "Cannot upsert asset with empty source path");
    
    std::unique_lock lock(m_mutex);
    m_assets[info.guid] = info;
    
    if (!info.sourcePath.empty())
    {
        m_sourceToGuid[NormalizePath(info.sourcePath)] = info.guid;
    }
}

bool AssetDatabase::remove(const AssetID& guid)
{
    if (guid.empty())
        return false;
    
    std::unique_lock lock(m_mutex);
    auto it = m_assets.find(guid);
    if (it == m_assets.end())
        return false;
    
    if (!it->second.sourcePath.empty())
    {
        m_sourceToGuid.erase(NormalizePath(it->second.sourcePath));
    }
    m_assets.erase(it);
    return true;
}

std::optional<AssetInfo> AssetDatabase::get(const AssetID& guid) const
{
    if (guid.empty())
        return std::nullopt;
    
    std::shared_lock lock(m_mutex);
    if (auto it = m_assets.find(guid); it != m_assets.end())
        return it->second;
    return std::nullopt;
}

std::optional<AssetInfo> AssetDatabase::findBySource(const std::string& srcPath) const
{
    LT_ASSERT_MSG(!srcPath.empty(), "Source path cannot be empty");
    
    std::shared_lock lock(m_mutex);
    std::string norm = NormalizePath(std::filesystem::path(srcPath).generic_string());
    auto it          = m_sourceToGuid.find(norm);
    if (it == m_sourceToGuid.end())
        return std::nullopt;
    
    auto jt = m_assets.find(it->second);
    if (jt == m_assets.end())
    {
        LT_LOGE("AssetDatabase", "Source path found but GUID not found in assets map");
        return std::nullopt;
    }
    return jt->second;
}

void AssetDatabase::clear()
{
    std::unique_lock lock(m_mutex);
    m_assets.clear();
    m_sourceToGuid.clear();
}

size_t AssetDatabase::size() const
{
    std::shared_lock lock(m_mutex);
    return m_assets.size();
}
