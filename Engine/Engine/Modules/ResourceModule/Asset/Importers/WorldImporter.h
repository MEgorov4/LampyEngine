#pragma once
#include "../IAssetImporter.h"
#include <Foundation/Assert/Assert.h>
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
        LT_ASSERT_MSG(!sourcePath.empty(), "Source path cannot be empty");
        LT_ASSERT_MSG(!cacheRoot.empty(), "Cache root cannot be empty");
        LT_ASSERT_MSG(std::filesystem::exists(sourcePath), "Source file does not exist: " + sourcePath.string());
        LT_ASSERT_MSG(std::filesystem::is_regular_file(sourcePath), "Source path is not a file");
        
        AssetInfo info{};
        info.type       = AssetType::World;
        info.sourcePath = sourcePath.string();
        info.guid       = MakeDeterministicIDFromPath(std::filesystem::path(sourcePath).generic_string());
        LT_ASSERT_MSG(!info.guid.empty(), "Generated GUID is empty");

        std::ifstream ifs(sourcePath);
        if (!ifs.is_open())
            throw std::runtime_error("Cannot open world file: " + sourcePath.string());

        std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        
        LT_ASSERT_MSG(!json.empty(), "World JSON file is empty");
        LT_ASSERT_MSG(json.size() < 100 * 1024 * 1024, "World JSON file is unreasonably large"); // 100MB limit

        std::filesystem::path outDir = cacheRoot / "Worlds";
        std::filesystem::create_directories(outDir);
        LT_ASSERT_MSG(std::filesystem::exists(outDir), "Failed to create Worlds directory");
        
        std::filesystem::path outFile = outDir / (sourcePath.stem().string() + ".worldbin");
        LT_ASSERT_MSG(!outFile.empty(), "Output file path is empty");

        std::ofstream ofs(outFile, std::ios::binary | std::ios::trunc);
        LT_ASSERT_MSG(ofs.is_open(), "Failed to open output file: " + outFile.string());
        
        uint32_t size = static_cast<uint32_t>(json.size());
        LT_ASSERT_MSG(size > 0, "World JSON size is zero");
        LT_ASSERT_MSG(size == json.size(), "World JSON size overflow");
        
        ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));
        ofs.write(json.data(), json.size());
        ofs.close();
        
        LT_ASSERT_MSG(std::filesystem::exists(outFile), "Output file was not created");
        LT_ASSERT_MSG(std::filesystem::file_size(outFile) > 0, "Output file is empty");

        info.importedPath      = outFile.string();
        info.sourceFileSize    = std::filesystem::file_size(sourcePath);
        info.importedFileSize  = std::filesystem::file_size(outFile);
        info.sourceTimestamp   = std::filesystem::last_write_time(sourcePath).time_since_epoch().count();
        info.importedTimestamp = std::filesystem::last_write_time(outFile).time_since_epoch().count();
        
        LT_ASSERT_MSG(!info.importedPath.empty(), "Imported path is empty");
        LT_ASSERT_MSG(info.importedFileSize > 0, "Imported file size is zero");

        LT_LOGI("WorldImporter", "Imported world: " + sourcePath.string());
        return info;
    }
};
}
