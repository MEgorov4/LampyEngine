#pragma once

#include "../IAssetWriter.h"

namespace ResourceModule
{

class WorldWriter final : public IAssetWriter
{
  public:
    bool supportsExtension(const std::string &ext) const noexcept override;
    AssetType getAssetType() const noexcept override;
    bool write(const BaseResource &resource, const std::filesystem::path &targetPath,
               const WriterContext &context) override;

  private:
    [[nodiscard]] bool writeToTemporary(const std::filesystem::path &tmpPath, const std::string &jsonData) const;
};

} // namespace ResourceModule



