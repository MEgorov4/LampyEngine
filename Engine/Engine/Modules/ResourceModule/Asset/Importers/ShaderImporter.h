#pragma once
#include "../IAssetImporter.h"
#include <Foundation/Assert/Assert.h>
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
            LT_ASSERT_MSG(!sourcePath.empty(), "Source path cannot be empty");
            LT_ASSERT_MSG(!cacheRoot.empty(), "Cache root cannot be empty");
            LT_ASSERT_MSG(std::filesystem::exists(sourcePath), "Source file does not exist: " + sourcePath.string());
            LT_ASSERT_MSG(std::filesystem::is_regular_file(sourcePath), "Source path is not a file");
            
            AssetInfo info{};
            info.type = AssetType::Shader;
            info.sourcePath = sourcePath.string();
            info.guid       = MakeDeterministicIDFromPath(std::filesystem::path(sourcePath).generic_string());
            LT_ASSERT_MSG(!info.guid.empty(), "Generated GUID is empty");

            std::filesystem::path outDir = cacheRoot / "Shaders";
            std::filesystem::create_directories(outDir);
            LT_ASSERT_MSG(std::filesystem::exists(outDir), "Failed to create Shaders directory");
            
            std::filesystem::path outFile = outDir / sourcePath.filename();
            LT_ASSERT_MSG(!outFile.empty(), "Output file path is empty");

            // просто копируем исходник в Cache/
            std::error_code ec;
            std::filesystem::copy_file(sourcePath, outFile,
                std::filesystem::copy_options::overwrite_existing, ec);
            
            if (ec)
            {
                throw std::runtime_error("Failed to copy shader file: " + ec.message());
            }
            
            LT_ASSERT_MSG(std::filesystem::exists(outFile), "Output file was not created");
            LT_ASSERT_MSG(std::filesystem::file_size(outFile) > 0, "Output file is empty");

            info.importedPath = outFile.string();
            info.sourceFileSize = std::filesystem::file_size(sourcePath);
            info.importedFileSize = std::filesystem::file_size(outFile);
            info.sourceTimestamp = std::filesystem::last_write_time(sourcePath).time_since_epoch().count();
            info.importedTimestamp = std::filesystem::last_write_time(outFile).time_since_epoch().count();
            
            LT_ASSERT_MSG(!info.importedPath.empty(), "Imported path is empty");
            LT_ASSERT_MSG(info.importedFileSize > 0, "Imported file size is zero");
            
            return info;
        }
    };
}
