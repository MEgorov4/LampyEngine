// PakBuilder.cpp
#include "PakBuilder.h"

#include "PakEntry.h"

#include <EngineMinimal.h>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace ResourceModule;
using EngineCore::Foundation::ResourceAllocator;

bool PakBuilder::BuildPak(const AssetDatabase& db, const std::filesystem::path& cacheDir,
                          const std::filesystem::path& outPakPath)
{
    LT_LOGI("PakBuilder", "Building PAK: " + outPakPath.string());

    PakHeader header{};
    nlohmann::json indexJson = nlohmann::json::object();

    std::ofstream ofs(outPakPath, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
    {
        LT_LOGE("PakBuilder", "Cannot open output file: " + outPakPath.string());
        return false;
    }

    ofs.write(reinterpret_cast<const char*>(&header), sizeof(header));
    uint64_t offset = sizeof(PakHeader);

    db.forEach(
        [&](const AssetID& guid, const AssetInfo& info)
        {
            const auto& imported = info.importedPath;
            if (imported.empty())
                return;

            std::ifstream in(imported, std::ios::binary);
            if (!in.is_open())
            {
                LT_LOGW("PakBuilder", "Skip missing imported: " + imported);
                return;
            }

            in.seekg(0, std::ios::end);
            const uint64_t size = static_cast<uint64_t>(in.tellg());
            in.seekg(0, std::ios::beg);

            std::vector<char, ResourceAllocator<char>> buffer(size);
            if (size)
                in.read(buffer.data(), size);

            ofs.write(buffer.data(), size);

            nlohmann::json e;
            e["offset"] = offset;
            e["size"]   = size;
            e["type"]   = static_cast<int>(info.type);
            e["path"]   = imported;

            indexJson[guid.str()] = std::move(e);
            offset += size;
        });

    const std::string indexStr = indexJson.dump();
    const uint64_t indexOffset = offset;
    const uint64_t indexSize   = static_cast<uint64_t>(indexStr.size());

    ofs.write(indexStr.data(), indexSize);

    PakHeader finalHeader{};
    finalHeader.indexOffset = indexOffset;
    finalHeader.indexSize   = indexSize;

    ofs.seekp(0, std::ios::beg);
    ofs.write(reinterpret_cast<const char*>(&finalHeader), sizeof(finalHeader));
    ofs.close();

    LT_LOGI("PakBuilder", "Pak built successfully: " + outPakPath.string());
    return true;
}
