#pragma once
#include "Foundation/Memory/ResourceAllocator.h"
#include "IAssetImporter.h"
#include <Foundation/Assert/Assert.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

using EngineCore::Foundation::ResourceAllocator;

namespace ResourceModule
{
class AssetImporterHub
{
  public:
    using ImporterPtr = std::unique_ptr<IAssetImporter, std::function<void(IAssetImporter*)>>;
    
    template<typename Deleter = std::default_delete<IAssetImporter>>
    void registerImporter(std::unique_ptr<IAssetImporter, Deleter> importer)
    {
        LT_ASSERT_MSG(importer, "Cannot register null importer");
        LT_ASSERT_MSG(importer->getAssetType() != AssetType::Unknown, "Cannot register importer with Unknown type");
        
        // Get the deleter BEFORE releasing the pointer
        Deleter originalDeleter = importer.get_deleter();
        
        // Release the pointer
        IAssetImporter* rawPtr = importer.release();
        
        // Create a std::function deleter that wraps the original deleter
        std::function<void(IAssetImporter*)> deleterFunc = [originalDeleter](IAssetImporter* p) mutable {
            if (p)
            {
                originalDeleter(p);
            }
        };
        
        // Store with custom deleter using std::function
        m_importers.emplace_back(ImporterPtr(rawPtr, deleterFunc));
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
    std::vector<ImporterPtr, ResourceAllocator<ImporterPtr>> m_importers;
};
} // namespace ResourceModule
