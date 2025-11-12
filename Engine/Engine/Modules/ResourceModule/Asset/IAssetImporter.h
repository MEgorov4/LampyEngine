#pragma once
#include "AssetInfo.h"

#include <EngineMinimal.h>

namespace ResourceModule
{
    class IAssetImporter
    {
    public:
        virtual ~IAssetImporter() = default;

        virtual bool supportsExtension(const std::string& ext) const noexcept = 0;

        virtual AssetType getAssetType() const noexcept = 0;

        virtual AssetInfo import(const std::filesystem::path& sourcePath,
                                 const std::filesystem::path& cacheRoot) = 0;
    };
}
