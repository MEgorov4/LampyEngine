#pragma once
#include "Foundation/Profiler/ProfileAllocator.h"
#include "IAssetImporter.h"
#include <EngineContext/Foundation/Assert/Assert.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace ResourceModule
{
class AssetImporterHub
{
  public:
    void registerImporter(std::unique_ptr<IAssetImporter> importer)
    {
        LT_ASSERT_MSG(importer, "Cannot register null importer");
        LT_ASSERT_MSG(importer->getAssetType() != AssetType::Unknown, "Cannot register importer with Unknown type");
        
        m_importers.emplace_back(std::move(importer));
    }

    IAssetImporter* findImporter(const std::string& ext) const noexcept
    {
        LT_ASSERT_MSG(!ext.empty(), "Extension cannot be empty");
        
        auto it = std::find_if(m_importers.begin(), m_importers.end(),
                               [&](const auto& ptr) { return ptr->supportsExtension(ext); });
        return it != m_importers.end() ? it->get() : nullptr;
    }

    IAssetImporter* findImporter(AssetType type) const noexcept
    {
        LT_ASSERT_MSG(type != AssetType::Unknown, "Cannot find importer for Unknown type");
        
        auto it = std::find_if(m_importers.begin(), m_importers.end(),
                               [&](const auto& ptr) { return ptr->getAssetType() == type; });
        return it != m_importers.end() ? it->get() : nullptr;
    }

  private:
    std::vector<std::unique_ptr<IAssetImporter>, ProfileAllocator<std::unique_ptr<IAssetImporter>>> m_importers;
};
} // namespace ResourceModule
