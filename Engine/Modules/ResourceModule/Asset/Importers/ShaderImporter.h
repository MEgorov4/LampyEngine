#pragma once
#include "../IAssetImporter.h"
#include <filesystem>
#include <fstream>

namespace ResourceModule
{
    class ShaderImporter final : public IAssetImporter
    {
    public:
        bool supportsExtension(const std::string& ext) const noexcept override
        {
            return ext == ".vert" || ext == ".frag";
        }

        AssetType getAssetType() const noexcept override { return AssetType::Shader; }

        AssetInfo import(const std::filesystem::path& sourcePath,
                         const std::filesystem::path& cacheRoot) override
        {
            AssetInfo info{};
            info.type = AssetType::Shader;
            info.sourcePath = sourcePath.string();
            info.guid       = MakeDeterministicIDFromPath(std::filesystem::path(sourcePath).generic_string());

            std::filesystem::path outDir = cacheRoot / "Shaders";
            std::filesystem::create_directories(outDir);
            std::filesystem::path outFile = outDir / sourcePath.filename();

            // просто копируем исходник в Cache/
            std::filesystem::copy_file(sourcePath, outFile,
                std::filesystem::copy_options::overwrite_existing);

            info.importedPath = outFile.string();
            info.sourceFileSize = std::filesystem::file_size(sourcePath);
            info.importedFileSize = std::filesystem::file_size(outFile);
            info.sourceTimestamp = std::filesystem::last_write_time(sourcePath).time_since_epoch().count();
            info.importedTimestamp = std::filesystem::last_write_time(outFile).time_since_epoch().count();
            return info;
        }
    };
}
