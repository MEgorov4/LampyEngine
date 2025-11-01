#pragma once
#include "IAssetImporter.h"
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

namespace ResourceModule
{
    class AssetImporterHub
    {
    public:
        void registerImporter(std::unique_ptr<IAssetImporter> importer)
        {
            m_importers.emplace_back(std::move(importer));
        }

        IAssetImporter* findImporter(const std::string& ext) const noexcept
        {
            auto it = std::find_if(m_importers.begin(), m_importers.end(),
                [&](const auto& ptr) { return ptr->supportsExtension(ext); });
            return it != m_importers.end() ? it->get() : nullptr;
        }

        IAssetImporter* findImporter(AssetType type) const noexcept
        {
            auto it = std::find_if(m_importers.begin(), m_importers.end(),
                [&](const auto& ptr) { return ptr->getAssetType() == type; });
            return it != m_importers.end() ? it->get() : nullptr;
        }

    private:
        std::vector<std::unique_ptr<IAssetImporter>> m_importers;
    };
}
