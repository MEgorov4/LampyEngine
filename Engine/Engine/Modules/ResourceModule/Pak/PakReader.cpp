#include "PakReader.h"

#include <EngineMinimal.h>

using namespace ResourceModule;
using EngineCore::Foundation::ResourceAllocator;

PakReader::PakReader(const std::string& pakPath)
{
    m_stream.open(pakPath, std::ios::binary);
    if (!m_stream.is_open())
    {
        LT_LOGE("PakReader", "Failed to open pak: " + pakPath);
        return;
    }

    m_stream.read(reinterpret_cast<char*>(&m_header), sizeof(PakHeader));
    if (strncmp(m_header.magic, "LPAK", 4) != 0)
    {
        LT_LOGE("PakReader", "Invalid PAK magic header!");
        m_stream.close();
        return;
    }

    loadIndex();
}

void PakReader::loadIndex()
{
    if (!m_stream.is_open())
        return;

    m_stream.seekg(m_header.indexOffset, std::ios::beg);
    std::vector<char, ResourceAllocator<char>> buffer(m_header.indexSize);
    m_stream.read(buffer.data(), m_header.indexSize);

    nlohmann::json j = nlohmann::json::parse(buffer.begin(), buffer.end());
    for (auto& [guidStr, val] : j.items())
    {
        PakEntry entry{};
        entry.guid = AssetID(guidStr);
        val.at("offset").get_to(entry.offset);
        val.at("size").get_to(entry.size);
        if (val.contains("type"))
            val.at("type").get_to(entry.type);
        if (val.contains("path"))
            val.at("path").get_to(entry.path);

        m_index.emplace(entry.guid, std::move(entry));
    }
}

bool PakReader::exists(const AssetID& guid) const noexcept
{
    return m_index.find(guid) != m_index.end();
}

std::optional<std::vector<uint8_t, ResourceAllocator<uint8_t>>> PakReader::readAsset(const AssetID& guid)
{
    if (!isOpen())
        return std::nullopt;

    auto it = m_index.find(guid);
    if (it == m_index.end())
        return std::nullopt;

    const PakEntry& e = it->second;
    m_stream.seekg(e.offset, std::ios::beg);

    std::vector<uint8_t, ResourceAllocator<uint8_t>> data(e.size);
    m_stream.read(reinterpret_cast<char*>(data.data()), e.size);
    return data;
}

std::vector<AssetID> PakReader::listAll() const
{
    std::vector<AssetID> ids;
    ids.reserve(m_index.size());
    for (auto& [id, _] : m_index)
        ids.push_back(id);
    return ids;
}
