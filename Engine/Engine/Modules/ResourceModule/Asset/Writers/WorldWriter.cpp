#include "WorldWriter.h"

#include "../../RWorld.h"
#include <Foundation/Assert/Assert.h>
#include <filesystem>
#include <fstream>
#include <format>

using namespace ResourceModule;

namespace
{
constexpr uint32_t kMaxWorldSizeBytes = 100 * 1024 * 1024;
}

bool WorldWriter::supportsExtension(const std::string &ext) const noexcept
{
    return ext == ".worldbin" || ext == ".lworld";
}

AssetType WorldWriter::getAssetType() const noexcept
{
    return AssetType::World;
}

bool WorldWriter::write(const BaseResource &resource, const std::filesystem::path &targetPath,
                        const WriterContext &context)
{
    LT_ASSERT_MSG(!targetPath.empty(), "Target path cannot be empty for WorldWriter");
    (void)context;

    auto world = dynamic_cast<const RWorld *>(&resource);
    if (!world)
    {
        LT_LOGE("WorldWriter", "Resource is not an RWorld");
        return false;
    }

    const std::string &jsonData = world->getJsonData();
    if (jsonData.empty())
    {
        LT_LOGE("WorldWriter", "World JSON data is empty, aborting write");
        return false;
    }

    if (jsonData.size() >= kMaxWorldSizeBytes)
    {
        LT_LOGE("WorldWriter", std::format("World JSON data is too large ({} bytes)", jsonData.size()));
        return false;
    }

    std::filesystem::path parent = targetPath.parent_path();
    if (!parent.empty())
    {
        std::error_code dirEc;
        std::filesystem::create_directories(parent, dirEc);
        if (dirEc)
        {
            LT_LOGE("WorldWriter", "Failed to create target directories: " + dirEc.message());
            return false;
        }
    }

    std::filesystem::path tmpPath = targetPath;
    tmpPath += ".tmp";

    if (!writeToTemporary(tmpPath, jsonData))
        return false;

    std::error_code renameEc;
    std::filesystem::rename(tmpPath, targetPath, renameEc);
    if (renameEc)
    {
        LT_LOGE("WorldWriter", "Failed to move temporary file into place: " + renameEc.message());
        std::filesystem::remove(tmpPath);
        return false;
    }

    LT_LOGI("WorldWriter", "Saved world to: " + targetPath.string());
    return true;
}

bool WorldWriter::writeToTemporary(const std::filesystem::path &tmpPath, const std::string &jsonData) const
{
    std::ofstream ofs(tmpPath, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
    {
        LT_LOGE("WorldWriter", "Failed to open temp file for writing: " + tmpPath.string());
        return false;
    }

    uint32_t size = static_cast<uint32_t>(jsonData.size());
    if (size == 0 || size != jsonData.size())
    {
        LT_LOGE("WorldWriter", "World JSON size overflow or zero");
        return false;
    }

    ofs.write(reinterpret_cast<const char *>(&size), sizeof(size));
    ofs.write(jsonData.data(), jsonData.size());
    ofs.flush();

    if (!ofs.good())
    {
        LT_LOGE("WorldWriter", "Failed to write world data to temp file: " + tmpPath.string());
        ofs.close();
        std::filesystem::remove(tmpPath);
        return false;
    }

    ofs.close();
    return true;
}


