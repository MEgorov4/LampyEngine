#include "AssetDatabase.h"

#include <filesystem>
#include <fstream>

using namespace ResourceModule;

static std::string NormalizePath(std::string path)
{
#ifdef _WIN32
    for (auto &c : path)
    {
        if (c == '/')
            c = '\\';
        else
            c = (char)tolower(c);
    }
#else
    for (auto &c : path)
        if (c == '\\')
            c = '/';
#endif
    return path;
}

std::vector<AssetInfo> AssetDatabase::getByOrigin(AssetOrigin origin) const
{
    std::shared_lock lock(m_mutex);
    std::vector<AssetInfo> result;
    result.reserve(m_assets.size());
    for (const auto &[_, info] : m_assets)
        if (info.origin == origin)
            result.push_back(info);
    return result;
}

bool AssetDatabase::load(const std::string &path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
        return false;

    nlohmann::json j;
    ifs >> j;

    std::unique_lock lock(m_mutex);
    m_assets.clear();
    m_sourceToGuid.clear();

    for (auto &[guidStr, infoJson] : j.items())
    {
        AssetInfo info = infoJson.get<AssetInfo>();
        m_sourceToGuid[NormalizePath(info.sourcePath)] = info.guid;
        m_assets.emplace(info.guid, std::move(info));
    }
    return true;
}

bool AssetDatabase::save(const std::string &path) const
{
    nlohmann::json j;
    {
        std::shared_lock lock(m_mutex);
        for (const auto &[guid, info] : m_assets)
            j[guid.str()] = info;
    }

    std::ofstream ofs(path);
    if (!ofs.is_open())
        return false;
    ofs << j.dump(2);
    return true;
}

void AssetDatabase::upsert(const AssetInfo &info)
{
    std::unique_lock lock(m_mutex);
    m_assets[info.guid] = info;
    m_sourceToGuid[NormalizePath(info.sourcePath)] = info.guid;
}

bool AssetDatabase::remove(const AssetID &guid)
{
    std::unique_lock lock(m_mutex);
    auto it = m_assets.find(guid);
    if (it == m_assets.end())
        return false;
    m_sourceToGuid.erase(NormalizePath(it->second.sourcePath));
    m_assets.erase(it);
    return true;
}

std::optional<AssetInfo> AssetDatabase::get(const AssetID &guid) const
{
    std::shared_lock lock(m_mutex);
    if (auto it = m_assets.find(guid); it != m_assets.end())
        return it->second;
    return std::nullopt;
}

std::optional<AssetInfo> AssetDatabase::findBySource(const std::string &srcPath) const
{
    std::shared_lock lock(m_mutex);
    std::string norm = NormalizePath(std::filesystem::path(srcPath).generic_string());
    auto it = m_sourceToGuid.find(norm);
    if (it == m_sourceToGuid.end())
        return std::nullopt;
    auto jt = m_assets.find(it->second);
    if (jt == m_assets.end())
        return std::nullopt;
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
