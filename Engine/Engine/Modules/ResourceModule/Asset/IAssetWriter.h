#pragma once

#include "AssetInfo.h"
#include <filesystem>
#include <string>

namespace ResourceModule
{
class AssetDatabase;
class ResourceRegistry;
class BaseResource;

struct WriterContext
{
    AssetDatabase *database = nullptr;
    ResourceRegistry *registry = nullptr;
    std::filesystem::path engineResourcesRoot;
    std::filesystem::path projectResourcesRoot;
    std::filesystem::path cacheRoot;
    bool usePak = false;

    [[nodiscard]] bool isValid() const noexcept
    {
        return database != nullptr;
    }
};

class IAssetWriter
{
  public:
    virtual ~IAssetWriter() = default;

    virtual bool supportsExtension(const std::string &ext) const noexcept = 0;
    virtual AssetType getAssetType() const noexcept                       = 0;
    virtual bool write(const BaseResource &resource, const std::filesystem::path &targetPath,
                       const WriterContext &context) = 0;
};

} // namespace ResourceModule



