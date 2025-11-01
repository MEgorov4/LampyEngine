#pragma once
#include "../IAssetImporter.h"
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
            AssetInfo info{};
            info.type = AssetType::Texture;
            info.sourcePath = sourcePath.string();
            info.guid       = MakeDeterministicIDFromPath(std::filesystem::path(sourcePath).generic_string());

            int w, h, c;
            stbi_uc* data = stbi_load(sourcePath.string().c_str(), &w, &h, &c, STBI_rgb_alpha);
            if (!data)
                throw std::runtime_error("Failed to load texture: " + sourcePath.string());

            const size_t dataSize = static_cast<size_t>(w * h * 4);

            std::filesystem::path outFile = cacheRoot / "Textures";
            std::filesystem::create_directories(outFile);
            outFile /= (sourcePath.stem().string() + ".texbin");

            std::ofstream ofs(outFile, std::ios::binary | std::ios::trunc);
            ofs.write(reinterpret_cast<const char*>(&w), sizeof(int));
            ofs.write(reinterpret_cast<const char*>(&h), sizeof(int));
            ofs.write(reinterpret_cast<const char*>(&c), sizeof(int));
            ofs.write(reinterpret_cast<const char*>(data), dataSize);
            ofs.close();
            stbi_image_free(data);

            info.importedPath = outFile.string();
            info.sourceFileSize = std::filesystem::file_size(sourcePath);
            info.importedFileSize = std::filesystem::file_size(outFile);
            info.sourceTimestamp = std::filesystem::last_write_time(sourcePath).time_since_epoch().count();
            info.importedTimestamp = std::filesystem::last_write_time(outFile).time_since_epoch().count();

            return info;
        }
    };
}
