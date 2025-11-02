#pragma once
#include "../IAssetImporter.h"
#include "Foundation/Profiler/ProfileAllocator.h"
#include <EngineContext/Foundation/Assert/Assert.h>
#include <stb_image.h>
#include <fstream>

namespace ResourceModule
{
    class TextureImporter final : public IAssetImporter
    {
    public:
        bool supportsExtension(const std::string& ext) const noexcept override
        {
            return ext == ".png" || ext == ".jpg" || ext == ".jpeg";
        }

        AssetType getAssetType() const noexcept override { return AssetType::Texture; }

        AssetInfo import(const std::filesystem::path& sourcePath,
                         const std::filesystem::path& cacheRoot) override
        {
            LT_ASSERT_MSG(!sourcePath.empty(), "Source path cannot be empty");
            LT_ASSERT_MSG(!cacheRoot.empty(), "Cache root cannot be empty");
            LT_ASSERT_MSG(std::filesystem::exists(sourcePath), "Source file does not exist: " + sourcePath.string());
            LT_ASSERT_MSG(std::filesystem::is_regular_file(sourcePath), "Source path is not a file");
            
            AssetInfo info{};
            info.type = AssetType::Texture;
            info.sourcePath = sourcePath.string();
            info.guid       = MakeDeterministicIDFromPath(std::filesystem::path(sourcePath).generic_string());
            LT_ASSERT_MSG(!info.guid.empty(), "Generated GUID is empty");

            int w, h, c;
            stbi_uc* data = stbi_load(sourcePath.string().c_str(), &w, &h, &c, STBI_rgb_alpha);
            if (!data)
                throw std::runtime_error("Failed to load texture: " + sourcePath.string());

            LT_ASSERT_MSG(w > 0, "Texture width must be positive");
            LT_ASSERT_MSG(h > 0, "Texture height must be positive");
            LT_ASSERT_MSG(w <= 16384, "Texture width is unreasonably large");
            LT_ASSERT_MSG(h <= 16384, "Texture height is unreasonably large");
            LT_ASSERT_MSG(c > 0 && c <= 4, "Invalid texture channel count");

            const size_t dataSize = static_cast<size_t>(w * h * 4);
            LT_ASSERT_MSG(dataSize > 0, "Texture data size is zero");

            std::filesystem::path outFile = cacheRoot / "Textures";
            std::filesystem::create_directories(outFile);
            LT_ASSERT_MSG(std::filesystem::exists(outFile), "Failed to create Textures directory");
            
            outFile /= (sourcePath.stem().string() + ".texbin");
            LT_ASSERT_MSG(!outFile.empty(), "Output file path is empty");

            std::ofstream ofs(outFile, std::ios::binary | std::ios::trunc);
            LT_ASSERT_MSG(ofs.is_open(), "Failed to open output file: " + outFile.string());
            
            ofs.write(reinterpret_cast<const char*>(&w), sizeof(int));
            ofs.write(reinterpret_cast<const char*>(&h), sizeof(int));
            ofs.write(reinterpret_cast<const char*>(&c), sizeof(int));
            ofs.write(reinterpret_cast<const char*>(data), dataSize);
            ofs.close();
            stbi_image_free(data);
            
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
