#pragma once

#include "../../ImGuiModule/GUIObject.h"
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include <vector>
#include <string>

namespace ResourceModule
{
    class AssetManager;
}

class GUIAssetBrowser : public ImGUIModule::GUIObject
{
public:
    GUIAssetBrowser();
    ~GUIAssetBrowser() override = default;

    void render(float deltaTime) override;

private:
    struct AssetEntry
    {
        std::string name;
        ResourceModule::AssetID assetID;
        ResourceModule::AssetType type;
        std::string sourcePath;
    };

    void refreshAssetList();
    void applyFilters();
    std::string getAssetTypeString(ResourceModule::AssetType type) const;
    std::string getAssetFileName(const std::string& sourcePath) const;

    ResourceModule::AssetManager* m_assetManager;
    std::vector<AssetEntry> m_allAssets;  // Все ресурсы
    std::vector<AssetEntry> m_filteredAssets;  // Отфильтрованные ресурсы
    
    // Фильтры
    char m_searchBuffer[256] = "";
    int m_selectedTypeFilter = 0;  // 0 = All, 1+ = конкретный тип
    
    bool m_needsRefresh = true;
    static constexpr const char* m_typeFilterOptions[] = {
        "All",
        "Texture",
        "Mesh", 
        "Shader",
        "Material",
        "Audio",
        "Script",
        "World",
        "Scene"
    };
};

