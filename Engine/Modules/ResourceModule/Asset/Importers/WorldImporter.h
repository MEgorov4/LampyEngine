#pragma once
#include "../IAssetImporter.h"
#include <fstream>

namespace ResourceModule
{
class WorldImporter final : public IAssetImporter
{
public:
    bool supportsExtension(const std::string& ext) const noexcept override
    {
        return ext == ".lworld";
    }

    AssetType getAssetType() const noexcept override
    {
        return AssetType::World;
    }

    AssetInfo import(const std::filesystem::path& sourcePath,
                     const std::filesystem::path& cacheRoot) override
    {
        AssetInfo info{};
        info.type       = AssetType::World;
        info.sourcePath = sourcePath.string();
        info.guid       = MakeDeterministicIDFromPath(std::filesystem::path(sourcePath).generic_string());;

        // читаем JSON-файл мира
        std::ifstream ifs(sourcePath);
        if (!ifs.is_open())
            throw std::runtime_error("Cannot open world file: " + sourcePath.string());

        std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        // создаём бинарь
        std::filesystem::path outDir = cacheRoot / "Worlds";
        std::filesystem::create_directories(outDir);
        std::filesystem::path outFile = outDir / (sourcePath.stem().string() + ".worldbin");

        std::ofstream ofs(outFile, std::ios::binary | std::ios::trunc);
        uint32_t size = static_cast<uint32_t>(json.size());
        ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));
        ofs.write(json.data(), json.size());
        ofs.close();

        info.importedPath      = outFile.string();
        info.sourceFileSize    = std::filesystem::file_size(sourcePath);
        info.importedFileSize  = std::filesystem::file_size(outFile);
        info.sourceTimestamp   = std::filesystem::last_write_time(sourcePath).time_since_epoch().count();
        info.importedTimestamp = std::filesystem::last_write_time(outFile).time_since_epoch().count();

        LT_LOGI("WorldImporter", "Imported world: " + sourcePath.string());
        return info;
    }
};
}
