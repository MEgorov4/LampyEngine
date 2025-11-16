#pragma once

#include "../IAssetWriter.h"
#include "../../Material.h"
#include <nlohmann/json.hpp>

namespace ResourceModule
{
class MaterialWriter final : public IAssetWriter
{
  public:
    bool supportsExtension(const std::string &ext) const noexcept override;
    AssetType getAssetType() const noexcept override;

    bool write(const BaseResource &resource, const std::filesystem::path &targetPath,
               const WriterContext &context) override;

  private:
    bool writeToTemporary(const std::filesystem::path &tmpPath, const nlohmann::json &payload) const;
};
} // namespace ResourceModule


