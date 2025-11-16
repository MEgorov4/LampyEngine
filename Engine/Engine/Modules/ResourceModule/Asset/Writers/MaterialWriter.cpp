#include "MaterialWriter.h"

#include <Foundation/Assert/Assert.h>
#include <Foundation/Log/LoggerMacro.h>
#include <filesystem>
#include <fstream>

using namespace ResourceModule;

bool MaterialWriter::supportsExtension(const std::string &ext) const noexcept
{
    return ext == ".lmat";
}

AssetType MaterialWriter::getAssetType() const noexcept
{
    return AssetType::Material;
}

bool MaterialWriter::write(const BaseResource &resource, const std::filesystem::path &targetPath,
                           const WriterContext &context)
{
    LT_ASSERT_MSG(!targetPath.empty(), "Target path cannot be empty for MaterialWriter");
    (void)context;

    auto material = dynamic_cast<const RMaterial *>(&resource);
    if (!material)
    {
        LT_LOGE("MaterialWriter", "Resource is not an RMaterial");
        return false;
    }

    nlohmann::json payload;
    const std::string guid = material->materialID.empty() ? material->getGUID() : material->materialID.str();
    payload["guid"]              = guid;
    payload["name"]              = material->name;
    payload["albedoColor"]       = {material->albedoColor.r, material->albedoColor.g, material->albedoColor.b,
                                    material->albedoColor.a};
    payload["emissiveColor"]     = {material->emissiveColor.r, material->emissiveColor.g, material->emissiveColor.b};
    payload["roughness"]         = material->roughness;
    payload["metallic"]          = material->metallic;
    payload["normalStrength"]    = material->normalStrength;
    payload["albedoTexture"]     = material->albedoTexture.str();
    payload["normalTexture"]     = material->normalTexture.str();
    payload["roughnessMetallicTexture"] = material->roughnessMetallicTexture.str();
    payload["emissiveTexture"]   = material->emissiveTexture.str();

    auto parent = targetPath.parent_path();
    if (!parent.empty())
    {
        std::error_code dirEc;
        std::filesystem::create_directories(parent, dirEc);
        if (dirEc)
        {
            LT_LOGE("MaterialWriter", "Failed to create directories: " + dirEc.message());
            return false;
        }
    }

    std::filesystem::path tmpPath = targetPath;
    tmpPath += ".tmp";
    if (!writeToTemporary(tmpPath, payload))
        return false;

    std::error_code renameEc;
    std::filesystem::rename(tmpPath, targetPath, renameEc);
    if (renameEc)
    {
        LT_LOGE("MaterialWriter", "Failed to move temporary file into place: " + renameEc.message());
        std::filesystem::remove(tmpPath);
        return false;
    }

    LT_LOGI("MaterialWriter", "Saved material to: " + targetPath.string());
    return true;
}

bool MaterialWriter::writeToTemporary(const std::filesystem::path &tmpPath, const nlohmann::json &payload) const
{
    std::ofstream ofs(tmpPath, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
    {
        LT_LOGE("MaterialWriter", "Failed to open temp file: " + tmpPath.string());
        return false;
    }

    ofs << payload.dump(4);
    ofs.flush();

    if (!ofs.good())
    {
        LT_LOGE("MaterialWriter", "Failed to write material data to temp file: " + tmpPath.string());
        ofs.close();
        std::filesystem::remove(tmpPath);
        return false;
    }

    ofs.close();
    return true;
}


