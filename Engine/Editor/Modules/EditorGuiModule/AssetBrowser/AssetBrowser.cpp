#include "AssetBrowser.h"

#include <imgui.h>
#include <Modules/ResourceModule/Asset/AssetManager.h>
#include <Modules/ResourceModule/Asset/AssetDatabase.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <filesystem>
#include <algorithm>
#include <cctype>

GUIAssetBrowser::GUIAssetBrowser() :
    ImGUIModule::GUIObject(),
    m_assetManager(GCM(ResourceModule::AssetManager))
{
}

void GUIAssetBrowser::render(float deltaTime)
{

    ZoneScopedN("GUIObject::AssetBrowser");
    if (!isVisible())
        return;

    if (m_needsRefresh)
    {
        refreshAssetList();
        applyFilters();
        m_needsRefresh = false;
    }

    bool windowOpen = true;
    if (ImGui::Begin("Asset Browser", &windowOpen, ImGuiWindowFlags_None))
    {
        // Header with refresh button
        if (ImGui::Button("↻ Refresh"))
        {
            refreshAssetList();
            applyFilters();
        }
        ImGui::SameLine();
        ImGui::Text("Total: %zu assets", m_allAssets.size());
        ImGui::SameLine();
        ImGui::Text("Filtered: %zu", m_filteredAssets.size());
        
        ImGui::Separator();

        // Filters
        ImGui::PushItemWidth(200.0f);
        if (ImGui::InputTextWithHint("##Search", "Search by name...", m_searchBuffer, sizeof(m_searchBuffer)))
        {
            applyFilters();
        }
        ImGui::PopItemWidth();
        
        ImGui::SameLine();
        ImGui::PushItemWidth(150.0f);
        if (ImGui::Combo("##TypeFilter", &m_selectedTypeFilter, m_typeFilterOptions, 
                         sizeof(m_typeFilterOptions) / sizeof(m_typeFilterOptions[0])))
        {
            applyFilters();
        }
        ImGui::PopItemWidth();

        ImGui::Separator();

        // Table with assets
        if (ImGui::BeginTable("AssetsTable", 3, 
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
        {
            // Column headers
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("AssetID", ImGuiTableColumnFlags_WidthFixed, 300.0f);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            // Asset rows (filtered)
            for (const auto& asset : m_filteredAssets)
            {
                ImGui::TableNextRow();
                
                // Name column with drag source
                ImGui::TableSetColumnIndex(0);
                
                // Make entire row selectable and draggable
                std::string uniqueID = asset.assetID.str() + asset.name;
                ImGui::PushID(uniqueID.c_str());
                
                // Create selectable area
                if (ImGui::Selectable(asset.name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
                {
                    // Selection logic can be added here if needed
                }
                
                // Drag and drop source for AssetID
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                {
                    std::string assetIDStr = asset.assetID.str();
                    ImGui::SetDragDropPayload("AssetID", assetIDStr.c_str(), assetIDStr.size() + 1);
                    ImGui::Text("%s", asset.name.c_str());
                    ImGui::Text("AssetID: %s", assetIDStr.c_str());
                    ImGui::Text("Type: %s", getAssetTypeString(asset.type).c_str());
                    ImGui::EndDragDropSource();
                }
                ImGui::PopID();

                // AssetID column
                ImGui::TableSetColumnIndex(1);
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "%s", asset.assetID.str().c_str());
                
                // Type column
                ImGui::TableSetColumnIndex(2);
                std::string typeStr = getAssetTypeString(asset.type);
                ImGui::TextColored(ImVec4(0.8f, 1.0f, 0.8f, 1.0f), "%s", typeStr.c_str());
            }

            ImGui::EndTable();
        }
    }
    
    // Handle window close button
    if (!windowOpen)
    {
        hide();
    }
    
    ImGui::End();
}

void GUIAssetBrowser::refreshAssetList()
{
    m_allAssets.clear();
    
    if (!m_assetManager)
        return;

    auto& database = m_assetManager->getDatabase();
    
    // Iterate through all assets using forEach
    database.forEach([this](const ResourceModule::AssetID& guid, const ResourceModule::AssetInfo& info)
    {
        AssetEntry entry{};
        entry.assetID = guid;
        entry.type = info.type;
        entry.sourcePath = info.sourcePath;
        entry.name = getAssetFileName(info.sourcePath);
        m_allAssets.push_back(entry);
    });
    
    // Sort by name
    std::sort(m_allAssets.begin(), m_allAssets.end(), 
        [](const AssetEntry& a, const AssetEntry& b) {
            return a.name < b.name;
        });
    
    applyFilters();
}

void GUIAssetBrowser::applyFilters()
{
    m_filteredAssets.clear();
    
    std::string searchLower = m_searchBuffer;
    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
    
    ResourceModule::AssetType selectedType = ResourceModule::AssetType::Unknown;
    if (m_selectedTypeFilter > 0)
    {
        switch (m_selectedTypeFilter - 1)
        {
            case 0: selectedType = ResourceModule::AssetType::Texture; break;
            case 1: selectedType = ResourceModule::AssetType::Mesh; break;
            case 2: selectedType = ResourceModule::AssetType::Shader; break;
            case 3: selectedType = ResourceModule::AssetType::Material; break;
            case 4: selectedType = ResourceModule::AssetType::Audio; break;
            case 5: selectedType = ResourceModule::AssetType::Script; break;
            case 6: selectedType = ResourceModule::AssetType::World; break;
        }
    }
    
    for (const auto& asset : m_allAssets)
    {
        // Filter by type
        if (m_selectedTypeFilter > 0 && asset.type != selectedType)
            continue;
        
        // Filter by name (case-insensitive)
        if (!searchLower.empty())
        {
            std::string assetNameLower = asset.name;
            std::transform(assetNameLower.begin(), assetNameLower.end(), assetNameLower.begin(), ::tolower);
            
            if (assetNameLower.find(searchLower) == std::string::npos)
                continue;
        }
        
        m_filteredAssets.push_back(asset);
    }
}

std::string GUIAssetBrowser::getAssetTypeString(ResourceModule::AssetType type) const
{
    switch (type)
    {
        case ResourceModule::AssetType::Texture: return "Texture";
        case ResourceModule::AssetType::Mesh: return "Mesh";
        case ResourceModule::AssetType::Shader: return "Shader";
        case ResourceModule::AssetType::Material: return "Material";
        case ResourceModule::AssetType::Audio: return "Audio";
        case ResourceModule::AssetType::Script: return "Script";
        case ResourceModule::AssetType::World: return "World";
        default: return "Unknown";
    }
}

std::string GUIAssetBrowser::getAssetFileName(const std::string& sourcePath) const
{
    std::filesystem::path path(sourcePath);
    return path.filename().string();
}

