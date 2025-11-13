#pragma once

#include <EngineMinimal.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ObjectCoreModule/ECS/Events.h>
#include <Modules/ObjectCoreModule/ECS/Systems/ECSLuaScriptsSystem.h>
#include <Modules/ObjectCoreModule/ECS/Systems/ECSPhysicsSystem.h>
#include <Modules/PhysicsModule/Components/RigidBodyComponent.h>
#include <Modules/PhysicsModule/Components/ColliderComponent.h>
#include <Modules/PhysicsModule/Components/CharacterControllerComponent.h>
#include <Modules/PhysicsModule/Components/PhysicsMaterialComponent.h>
#include <Modules/PhysicsModule/Utils/PhysicsTypes.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ResourceModule/Material.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <btBulletDynamicsCommon.h>
#include <flecs.h>
#include <imgui.h>
#include <filesystem>

class IComponentRenderer
{
  protected:
    ProjectModule::ProjectModule* m_projectModule;
    
    // Helper to get display name for AssetID (file name instead of UUID)
    std::string getAssetDisplayName(const ResourceModule::AssetID& assetID) const
    {
        if (assetID.empty())
            return "None";
        
        // Try to get AssetInfo from database
        if (auto* db = ResourceModule::AssetRegistryAccessor::Get())
        {
            if (auto infoOpt = db->get(assetID))
            {
                std::filesystem::path path(infoOpt->sourcePath);
                return path.filename().string();
            }
        }
        
        // Fallback to UUID if not found in database
        return assetID.str();
    }

  public:
    IComponentRenderer() :
        m_projectModule(GCXM(ProjectModule::ProjectModule))
    {
    }

    // Render component with header buttons (remove, reset) - override in derived classes if needed
    virtual void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName)
    {
        // Default: just call legacy render
        // Derived classes should override this and call renderComponentControls after CollapsingHeader
        render(entity);
    }

    // Legacy render method (for backward compatibility)
    virtual void render(flecs::entity& entity) = 0;

    // Render component header with controls (call after CollapsingHeader)
    void renderComponentControls(flecs::entity& entity, const std::string& typeName, const std::string& displayName)
    {
        ImGui::PushID((typeName + "_controls").c_str());
        
        // Position buttons at the end of the header line
        float buttonWidth = ImGui::CalcTextSize("↻ Reset").x + ImGui::GetStyle().FramePadding.x * 2;
        float buttonWidth2 = ImGui::CalcTextSize("× Remove").x + ImGui::GetStyle().FramePadding.x * 2;
        float totalWidth = buttonWidth + buttonWidth2 + ImGui::GetStyle().ItemSpacing.x;
        
        ImGui::SameLine(ImGui::GetWindowWidth() - totalWidth - 10);
        
        // Reset button
        if (ImGui::SmallButton("↻ Reset"))
        {
            Events::EditorUI::ComponentResetRequest evt{};
            evt.entityId = entity.id();
            evt.componentTypeName = typeName;
            GCEB().emit(evt);
        }
        
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Reset component to default values");
        }
        
        ImGui::SameLine();
        
        // Remove button (don't allow removing Position, Rotation, Scale - core transform)
        bool isCoreTransform = (typeName == "PositionComponent" || 
                                typeName == "RotationComponent" || 
                                typeName == "ScaleComponent");
        
        if (isCoreTransform)
        {
            ImGui::BeginDisabled(true);
        }
        
        if (ImGui::SmallButton("× Remove"))
        {
            Events::EditorUI::ComponentRemoveRequest evt{};
            evt.entityId = entity.id();
            evt.componentTypeName = typeName;
            GCEB().emit(evt);
        }
        
        if (isCoreTransform)
        {
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Core transform components cannot be removed");
            }
        }
        else
        {
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Remove component from entity");
            }
        }
        
        ImGui::PopID();
    }

    virtual ~IComponentRenderer()
    {
    }
};

class PositionRenderer : public IComponentRenderer
{
  public:
    PositionRenderer() : IComponentRenderer()
    {
    }

    void render(flecs::entity& entity) override
    {
        ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

        if (ImGui::BeginChildFrame(1, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5f,
                                             ImGui::GetWindowSize().y / 3.f)))
        {
            if (const PositionComponent* pos = entity.get<PositionComponent>())
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.3f, 0.5f));
                bool headerOpen = ImGui::CollapsingHeader("Position Component", ImGuiTreeNodeFlags_DefaultOpen);
                renderComponentControls(entity, "PositionComponent", "Position Component");
                
                if (headerOpen)
                {
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosX(0);
                    ImGui::Separator();

                    ImGui::Text("Position:");
                    ImGui::SameLine();

                    float position[3] = {pos->x, pos->y, pos->z};

                    if (ImGui::DragFloat3("##PositionComponent", position, 0.01f))
                    {
                        entity.set<PositionComponent>({position[0], position[1], position[2]});
                    }
                }
                else
                {
                    ImGui::PopStyleColor();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class RotationRenderer : public IComponentRenderer
{
  public:
    RotationRenderer() : IComponentRenderer()
    {
    }

    void render(flecs::entity& entity) override
    {
        ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

        if (ImGui::BeginChildFrame(
                1, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 3)))
        {
            if (auto rot = entity.get<RotationComponent>())
            {
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("RotationComponent").x) / 2);

                ImGui::SetCursorPosX(0);
                ImGui::Separator();

                ImGui::Text("RotationComponent");

                ImGui::SameLine();

                float rotation[3] = {rot->x, rot->y, rot->z};

                if (ImGui::DragFloat3("##RotationComponent", rotation, 5.f))
                {
                    RotationComponent newRot;
                    newRot.fromEulerDegrees(glm::vec3(rotation[0], rotation[1], rotation[2]));
                    entity.set<RotationComponent>(newRot);
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class ScaleRenderer : public IComponentRenderer
{
  public:
    ScaleRenderer() : IComponentRenderer()
    {
    }

    void render(flecs::entity& entity) override
    {
        ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

        if (ImGui::BeginChildFrame(
                1, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 3)))
        {
            if (const ScaleComponent* scale = entity.get<ScaleComponent>())
            {
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("ScaleComponent").x) / 2);

                ImGui::SetCursorPosX(0);
                ImGui::Separator();

                ImGui::Text("ScaleComponent");

                ImGui::SameLine();

                float scalev[3] = {scale->x, scale->y, scale->z};

                if (ImGui::DragFloat3("##ScaleComponent", scalev, 0.01f, -100, 100))
                {
                    auto sc = entity.get_mut<ScaleComponent>();
                    sc->fromGMLVec(glm::vec3(scalev[0], scalev[1], scalev[2]));
                    entity.modified<ScaleComponent>();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class MeshComponentRenderer : public IComponentRenderer
{
  public:
    MeshComponentRenderer() : IComponentRenderer()
    {
    }

    void render(flecs::entity& entity) override
    {
        if (ImGui::BeginChildFrame(
                2, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 4)))
        {
            if (const MeshComponent* meshComponent = entity.get<MeshComponent>())
            {
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("MeshComponent").x) / 2);

                ImGui::SetWindowFontScale(1.2);
                ImGui::Text("MeshComponent");
                ImGui::SetWindowFontScale(1);

                ImGui::SetCursorPosX(0);
                ImGui::Separator();

                std::string resPath = m_projectModule->getProjectConfig().getResourcesPath();
                
                // Mesh ID
                ImGui::Text("Mesh ID:");
                ImGui::SameLine();
                std::string meshDisplay = getAssetDisplayName(meshComponent->meshID);
                if (meshComponent->meshID.empty())
                    meshDisplay = "None (drop mesh asset)";
                ImGui::TextColored(meshComponent->meshID.empty() ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f) : ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "%s", meshDisplay.c_str());
                
                if (ImGui::BeginDragDropTarget())
                {
                    // Support AssetID from AssetBrowser
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetID"))
                    {
                        std::string assetIDStr = static_cast<const char*>(payload->Data);
                        ResourceModule::AssetID assetID(assetIDStr);
                        
                        MeshComponent* meshComponentMut = entity.get_mut<MeshComponent>();
                        if (meshComponentMut)
                        {
                            meshComponentMut->meshID = assetID;
                            entity.modified<MeshComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                        }
                    }
                    // Support FilePath from ContentBrowser
                    else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath"))
                    {
                        std::string droppedPath = static_cast<const char*>(payload->Data);
                        std::filesystem::path path(droppedPath);
                        std::string ext = path.extension().string();
                        
                        // Support .obj files for meshes
                        if (ext == ".obj" || ext == ".meshbin")
                        {
                            MeshComponent* meshComponentMut = entity.get_mut<MeshComponent>();
                            if (meshComponentMut)
                            {
                                // Convert absolute path to relative path if it's in resources
                                std::filesystem::path absPath(droppedPath);
                                std::filesystem::path relativePath;
                                
                                try {
                                    if (std::filesystem::exists(absPath))
                                    {
                                        std::filesystem::path canonicalPath = std::filesystem::canonical(absPath);
                                        std::filesystem::path resPathObj(resPath);
                                        if (canonicalPath.string().find(resPathObj.string()) == 0)
                                        {
                                            relativePath = std::filesystem::relative(canonicalPath, resPathObj);
                                            relativePath = relativePath.generic_string();
                                        }
                                        else
                                        {
                                            relativePath = canonicalPath.generic_string();
                                        }
                                    }
                                    else
                                    {
                                        relativePath = absPath.generic_string();
                                    }
                                }
                                catch (...)
                                {
                                    relativePath = absPath.generic_string();
                                }
                                
                                meshComponentMut->meshID = ResourceModule::AssetID(relativePath.generic_string());
                                entity.modified<MeshComponent>();
                                GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                // Texture ID
                ImGui::Text("Texture ID:");
                ImGui::SameLine();
                std::string textureDisplay = getAssetDisplayName(meshComponent->textureID);
                if (meshComponent->textureID.empty())
                    textureDisplay = "None (drop texture asset)";
                ImGui::TextColored(meshComponent->textureID.empty() ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f) : ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "%s", textureDisplay.c_str());
                
                if (ImGui::BeginDragDropTarget())
                {
                    // Support AssetID from AssetBrowser
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetID"))
                    {
                        std::string assetIDStr = static_cast<const char*>(payload->Data);
                        ResourceModule::AssetID assetID(assetIDStr);
                        
                        MeshComponent* meshComponentMut = entity.get_mut<MeshComponent>();
                        if (meshComponentMut)
                        {
                            meshComponentMut->textureID = assetID;
                            entity.modified<MeshComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                        }
                    }
                    // Support FilePath from ContentBrowser
                    else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath"))
                    {
                        std::string droppedPath = static_cast<const char*>(payload->Data);
                        std::filesystem::path path(droppedPath);
                        std::string ext = path.extension().string();
                        
                        // Support image files for textures
                        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                        {
                            MeshComponent* meshComponentMut = entity.get_mut<MeshComponent>();
                            if (meshComponentMut)
                            {
                                // Convert absolute path to relative path if it's in resources
                                std::filesystem::path absPath(droppedPath);
                                std::filesystem::path relativePath;
                                
                                try {
                                    if (std::filesystem::exists(absPath))
                                    {
                                        std::filesystem::path canonicalPath = std::filesystem::canonical(absPath);
                                        std::filesystem::path resPathObj(resPath);
                                        if (canonicalPath.string().find(resPathObj.string()) == 0)
                                        {
                                            relativePath = std::filesystem::relative(canonicalPath, resPathObj);
                                            relativePath = relativePath.generic_string();
                                        }
                                        else
                                        {
                                            relativePath = canonicalPath.generic_string();
                                        }
                                    }
                                    else
                                    {
                                        relativePath = absPath.generic_string();
                                    }
                                }
                                catch (...)
                                {
                                    relativePath = absPath.generic_string();
                                }
                                
                                meshComponentMut->textureID = ResourceModule::AssetID(relativePath.generic_string());
                                entity.modified<MeshComponent>();
                                GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                // Vertex Shader ID
                ImGui::Text("Vertex Shader ID:");
                ImGui::SameLine();
                std::string vertShaderDisplay = getAssetDisplayName(meshComponent->vertShaderID);
                if (meshComponent->vertShaderID.empty())
                    vertShaderDisplay = "None (drop shader asset)";
                ImGui::TextColored(meshComponent->vertShaderID.empty() ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f) : ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "%s", vertShaderDisplay.c_str());
                
                if (ImGui::BeginDragDropTarget())
                {
                    // Support both FilePath (from ContentBrowser) and AssetID (from AssetBrowser)
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetID"))
                    {
                        std::string assetIDStr = static_cast<const char*>(payload->Data);
                        ResourceModule::AssetID assetID(assetIDStr);
                        
                        MeshComponent* meshComponentMut = entity.get_mut<MeshComponent>();
                        if (meshComponentMut)
                        {
                            meshComponentMut->vertShaderID = assetID;
                            entity.modified<MeshComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                        }
                    }
                    else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath"))
                    {
                        std::string droppedPath = static_cast<const char*>(payload->Data);
                        std::filesystem::path path(droppedPath);
                        std::string ext = path.extension().string();
                        
                        // Support .vert files for vertex shaders
                        if (ext == ".vert")
                        {
                            MeshComponent* meshComponentMut = entity.get_mut<MeshComponent>();
                            if (meshComponentMut)
                            {
                                std::filesystem::path absPath(droppedPath);
                                std::filesystem::path relativePath;
                                
                                try {
                                    if (std::filesystem::exists(absPath))
                                    {
                                        std::filesystem::path canonicalPath = std::filesystem::canonical(absPath);
                                        std::filesystem::path resPathObj(resPath);
                                        if (canonicalPath.string().find(resPathObj.string()) == 0)
                                        {
                                            relativePath = std::filesystem::relative(canonicalPath, resPathObj);
                                            relativePath = relativePath.generic_string();
                                        }
                                        else
                                        {
                                            relativePath = canonicalPath.generic_string();
                                        }
                                    }
                                    else
                                    {
                                        relativePath = absPath.generic_string();
                                    }
                                }
                                catch (...)
                                {
                                    relativePath = absPath.generic_string();
                                }
                                
                                meshComponentMut->vertShaderID = ResourceModule::AssetID(relativePath.generic_string());
                                entity.modified<MeshComponent>();
                                GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                // Fragment Shader ID
                ImGui::Text("Fragment Shader ID:");
                ImGui::SameLine();
                std::string fragShaderDisplay = getAssetDisplayName(meshComponent->fragShaderID);
                if (meshComponent->fragShaderID.empty())
                    fragShaderDisplay = "None (drop shader asset)";
                ImGui::TextColored(meshComponent->fragShaderID.empty() ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f) : ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "%s", fragShaderDisplay.c_str());
                
                if (ImGui::BeginDragDropTarget())
                {
                    // Support AssetID from AssetBrowser
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetID"))
                    {
                        std::string assetIDStr = static_cast<const char*>(payload->Data);
                        ResourceModule::AssetID assetID(assetIDStr);
                        
                        MeshComponent* meshComponentMut = entity.get_mut<MeshComponent>();
                        if (meshComponentMut)
                        {
                            meshComponentMut->fragShaderID = assetID;
                            entity.modified<MeshComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                        }
                    }
                    // Support FilePath from ContentBrowser
                    else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath"))
                    {
                        std::string droppedPath = static_cast<const char*>(payload->Data);
                        std::filesystem::path path(droppedPath);
                        std::string ext = path.extension().string();
                        
                        // Support .frag files for fragment shaders
                        if (ext == ".frag")
                        {
                            MeshComponent* meshComponentMut = entity.get_mut<MeshComponent>();
                            if (meshComponentMut)
                            {
                                std::filesystem::path absPath(droppedPath);
                                std::filesystem::path relativePath;
                                
                                try {
                                    if (std::filesystem::exists(absPath))
                                    {
                                        std::filesystem::path canonicalPath = std::filesystem::canonical(absPath);
                                        std::filesystem::path resPathObj(resPath);
                                        if (canonicalPath.string().find(resPathObj.string()) == 0)
                                        {
                                            relativePath = std::filesystem::relative(canonicalPath, resPathObj);
                                            relativePath = relativePath.generic_string();
                                        }
                                        else
                                        {
                                            relativePath = canonicalPath.generic_string();
                                        }
                                    }
                                    else
                                    {
                                        relativePath = absPath.generic_string();
                                    }
                                }
                                catch (...)
                                {
                                    relativePath = absPath.generic_string();
                                }
                                
                                meshComponentMut->fragShaderID = ResourceModule::AssetID(relativePath.generic_string());
                                entity.modified<MeshComponent>();
                                GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class ScriptRenderer : public IComponentRenderer
{
  public:
    ScriptRenderer() : IComponentRenderer()
    {
    }

    void render(flecs::entity& entity) override
    {
        if (ImGui::BeginChildFrame(
                3, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 4)))
        {
            if (const ScriptComponent* script = entity.get<ScriptComponent>())
            {
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("ScriptComponent").x) / 2);

                ImGui::SetWindowFontScale(1.2);
                ImGui::Text("ScriptComponent");
                ImGui::SetWindowFontScale(1);

                ImGui::SetCursorPosX(0);
                ImGui::Separator();

                ImGui::Text("Path:");
                ImGui::SameLine();

                std::string resPath = m_projectModule->getProjectConfig().getResourcesPath();
                ImGui::Text(Fs::fileName(script->scriptPath.c_str()).empty()
                                ? "empty"
                                : Fs::fileName(script->scriptPath.c_str()).c_str());

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath"))
                    {
                        std::string droppedPath = static_cast<const char*>(payload->Data);
                        if (droppedPath.size() > 4 && droppedPath.substr(droppedPath.size() - 4) == ".lua")
                        {
                            entity.set<ScriptComponent>({droppedPath});
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class DirectionalLightRenderer : public IComponentRenderer
{
  public:
    DirectionalLightRenderer() : IComponentRenderer()
    {
    }

    void render(flecs::entity& entity) override
    {
        if (ImGui::BeginChildFrame(
                4, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 4)))
        {
            if (const DirectionalLightComponent* dirLightComponent = entity.get<DirectionalLightComponent>())
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.3f, 0.5f));
                bool headerOpen = ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen);
                renderComponentControls(entity, "DirectionalLightComponent", "Directional Light");
                
                if (headerOpen)
                {
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosX(0);
                    ImGui::Separator();

                    float intencity = dirLightComponent->intencity;
                    ImGui::Text("Intensity:");
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##dirLightIntencity", &intencity, 0.01f, 0.0f, 1000.f))
                    {
                        entity.set<DirectionalLightComponent>({intencity});
                        GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "DirectionalLightComponent"});
                    }
                }
                else
                {
                    ImGui::PopStyleColor();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class PointLightRenderer : public IComponentRenderer
{
  public:
    PointLightRenderer() : IComponentRenderer()
    {
    }

    void render(flecs::entity& entity) override
    {
        if (ImGui::BeginChildFrame(
                5, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 4)))
        {
            if (const PointLightComponent* pointLightComponent = entity.get<PointLightComponent>())
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.3f, 0.5f));
                bool headerOpen = ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen);
                renderComponentControls(entity, "PointLightComponent", "Point Light");
                
                if (headerOpen)
                {
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosX(0);
                    ImGui::Separator();

                    float innerRadius = pointLightComponent->innerRadius;
                    float outerRadius = pointLightComponent->outerRadius;
                    float intensity = pointLightComponent->intencity;
                    glm::vec3 color = pointLightComponent->color;
                    float colorArray[3] = { color.x, color.y, color.z };

                    ImGui::Text("Color:");
                    ImGui::SameLine();
                    if (ImGui::ColorEdit3("##pointLightColor", colorArray, ImGuiColorEditFlags_NoInputs))
                    {
                        PointLightComponent* comp = entity.get_mut<PointLightComponent>();
                        if (comp)
                        {
                            comp->color = glm::vec3(colorArray[0], colorArray[1], colorArray[2]);
                            entity.modified<PointLightComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "PointLightComponent"});
                        }
                    }

                    ImGui::Text("Intensity:");
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##pointLightIntensity", &intensity, 0.01f, 0.0f, 1000.f))
                    {
                        PointLightComponent* comp = entity.get_mut<PointLightComponent>();
                        if (comp)
                        {
                            comp->intencity = intensity;
                            entity.modified<PointLightComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "PointLightComponent"});
                        }
                    }

                    ImGui::Text("Inner Radius:");
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##pointLightInnerRadius", &innerRadius, 0.1f, 0.0f, 1000.f))
                    {
                        PointLightComponent* comp = entity.get_mut<PointLightComponent>();
                        if (comp)
                        {
                            comp->innerRadius = innerRadius;
                            if (comp->innerRadius > comp->outerRadius)
                                comp->innerRadius = comp->outerRadius;
                            entity.modified<PointLightComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "PointLightComponent"});
                        }
                    }

                    ImGui::Text("Outer Radius:");
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##pointLightOuterRadius", &outerRadius, 0.1f, 0.0f, 1000.f))
                    {
                        PointLightComponent* comp = entity.get_mut<PointLightComponent>();
                        if (comp)
                        {
                            comp->outerRadius = outerRadius;
                            if (comp->outerRadius < comp->innerRadius)
                                comp->outerRadius = comp->innerRadius;
                            entity.modified<PointLightComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "PointLightComponent"});
                        }
                    }
                }
                else
                {
                    ImGui::PopStyleColor();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class CameraRenderer : public IComponentRenderer
{
  public:
    CameraRenderer() : IComponentRenderer()
    {
    }

    void render(flecs::entity& entity) override
    {
        if (ImGui::BeginChildFrame(3, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5,
                                             ImGui::GetWindowSize().y / 2.25)))
        {
            if (CameraComponent* camera = entity.get_mut<CameraComponent>())
            {
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("CameraComponent").x) / 2);

                ImGui::SetWindowFontScale(1.2);
                ImGui::Text("CameraComponent");
                ImGui::SetWindowFontScale(1);

                ImGui::SetCursorPosX(0);
                ImGui::Separator();

                float fov      = camera->fov;
                float aspect   = camera->aspect;
                float farClip  = camera->farClip;
                float nearClip = camera->nearClip;

                const float labelWidth = 110.0f;

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Field of view:");
                ImGui::SameLine(labelWidth);
                if (ImGui::SliderFloat("##FOV", &fov, 30.0f, 180.0f))
                {
                    camera->fov = fov;
                    entity.modified<CameraComponent>();
                }

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Aspect ratio:");
                ImGui::SameLine(labelWidth);
                if (ImGui::SliderFloat("##ASPECT", &aspect, 0.01f, 1.0f))
                {
                    camera->aspect = aspect;
                    entity.modified<CameraComponent>();
                }

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Far clip:");
                ImGui::SameLine(labelWidth);
                if (ImGui::SliderFloat("##FAR", &farClip, 10.0f, 10000.0f))
                {
                    camera->farClip = farClip;
                    entity.modified<CameraComponent>();
                }

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Near clip:");
                ImGui::SameLine(labelWidth);
                if (ImGui::SliderFloat("##NEAR", &nearClip, 0.01f, 10.0f))
                {
                    camera->nearClip = nearClip;
                    entity.modified<CameraComponent>();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class RigidbodyRenderer : public IComponentRenderer
{
  public:
    RigidbodyRenderer() : IComponentRenderer()
    {
    }

    void render(flecs::entity& entity) override
    {
        ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

        if (ImGui::BeginChildFrame(
                5, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 3)))
        {
            if (RigidbodyComponent* body = entity.get_mut<RigidbodyComponent>())
            {
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("RigidbodyComponent").x) / 2);

                ImGui::SetWindowFontScale(1.2);
                ImGui::Text("RigidbodyComponent");
                ImGui::SetWindowFontScale(1);

                ImGui::SetCursorPosX(0);
                ImGui::Separator();

                float mass    = body->mass;
                bool isStatic = body->isStatic;

                ImGui::Text("static");
                ImGui::SameLine();

                if (ImGui::Checkbox("##static", &isStatic))
                {
                    body->isStatic = isStatic;
                }

                ImGui::Text("mass");
                ImGui::SameLine();

                ImGui::BeginDisabled(isStatic);
                if (ImGui::DragFloat("##mass", &mass, 0.1f, 0.f, 100000000.f))
                {
                    body->mass = mass;
                }
                ImGui::EndDisabled();
            }
        }
        ImGui::EndChildFrame();
    }
};

class MaterialRenderer : public IComponentRenderer
{
  public:
    MaterialRenderer() : IComponentRenderer()
    {
    }

    void render(flecs::entity& entity) override
    {
        if (ImGui::BeginChildFrame(
                6, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 4)))
        {
            if (const MaterialComponent* materialComponent = entity.get<MaterialComponent>())
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.3f, 0.5f));
                bool headerOpen = ImGui::CollapsingHeader("Material Component", ImGuiTreeNodeFlags_DefaultOpen);
                renderComponentControls(entity, "MaterialComponent", "Material Component");
                
                if (headerOpen)
                {
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosX(0);
                    ImGui::Separator();

                    // Material ID
                    ImGui::Text("Material ID:");
                    ImGui::SameLine();
                    std::string materialDisplay = getAssetDisplayName(materialComponent->materialID);
                    if (materialComponent->materialID.empty())
                        materialDisplay = "None (drop material asset)";
                    ImGui::TextColored(materialComponent->materialID.empty() ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f) : ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "%s", materialDisplay.c_str());
                    
                    if (ImGui::BeginDragDropTarget())
                    {
                        // Support AssetID from AssetBrowser
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetID"))
                        {
                            std::string assetIDStr = static_cast<const char*>(payload->Data);
                            ResourceModule::AssetID assetID(assetIDStr);
                            
                            // Check if it's a material asset
                            if (auto* db = ResourceModule::AssetRegistryAccessor::Get())
                            {
                                if (auto infoOpt = db->get(assetID))
                                {
                                    if (infoOpt->type == ResourceModule::AssetType::Material)
                                    {
                                        MaterialComponent* materialComponentMut = entity.get_mut<MaterialComponent>();
                                        if (materialComponentMut)
                                        {
                                            materialComponentMut->materialID = assetID;
                                            entity.modified<MaterialComponent>();
                                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MaterialComponent"});
                                        }
                                    }
                                }
                            }
                        }
                        // Support FilePath from ContentBrowser
                        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath"))
                        {
                            std::string droppedPath = static_cast<const char*>(payload->Data);
                            std::filesystem::path path(droppedPath);
                            std::string ext = path.extension().string();
                            
                            // Support .lmat files for materials
                            if (ext == ".lmat")
                            {
                                MaterialComponent* materialComponentMut = entity.get_mut<MaterialComponent>();
                                if (materialComponentMut)
                                {
                                    std::string resPath = m_projectModule->getProjectConfig().getResourcesPath();
                                    std::filesystem::path absPath(droppedPath);
                                    std::filesystem::path relativePath;
                                    
                                    try {
                                        if (std::filesystem::exists(absPath))
                                        {
                                            std::filesystem::path canonicalPath = std::filesystem::canonical(absPath);
                                            std::filesystem::path resPathObj(resPath);
                                            if (canonicalPath.string().find(resPathObj.string()) == 0)
                                            {
                                                relativePath = std::filesystem::relative(canonicalPath, resPathObj);
                                                relativePath = relativePath.generic_string();
                                            }
                                            else
                                            {
                                                relativePath = canonicalPath.generic_string();
                                            }
                                        }
                                        else
                                        {
                                            relativePath = absPath.generic_string();
                                        }
                                    }
                                    catch (...)
                                    {
                                        relativePath = absPath.generic_string();
                                    }
                                    
                                    materialComponentMut->materialID = ResourceModule::AssetID(relativePath.generic_string());
                                    entity.modified<MaterialComponent>();
                                    GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MaterialComponent"});
                                }
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                    
                    // Show material preview if loaded
                    if (!materialComponent->materialID.empty())
                    {
                        auto rm = GCM(ResourceModule::ResourceManager);
                        if (rm)
                        {
                            auto material = rm->load<ResourceModule::RMaterial>(materialComponent->materialID);
                            if (material)
                            {
                                ImGui::Separator();
                                ImGui::Text("Material: %s", material->name.c_str());
                                
                                // Albedo Color
                                float albedo[4] = {material->albedoColor.r, material->albedoColor.g, 
                                                  material->albedoColor.b, material->albedoColor.a};
                                ImGui::Text("Albedo Color:");
                                ImGui::SameLine();
                                if (ImGui::ColorEdit4("##AlbedoColor", albedo, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar))
                                {
                                    // Note: This would require saving the material back to file, which is not implemented yet
                                    // For now, this is read-only preview
                                }
                                
                                // Roughness
                                ImGui::Text("Roughness: %.2f", material->roughness);
                                
                                // Metallic
                                ImGui::Text("Metallic: %.2f", material->metallic);
                            }
                        }
                    }
                }
                else
                {
                    ImGui::PopStyleColor();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class ComponentRendererFactory
{
  public:
    using RendererCreator = std::function<std::unique_ptr<IComponentRenderer>()>;

    static ComponentRendererFactory& getInstance()
    {
        static ComponentRendererFactory instance;
        return instance;
    }

    void registerRenderer(const std::string& componentType, RendererCreator creator)
    {
        registry[componentType] = creator;
    }

    std::unique_ptr<IComponentRenderer> createRenderer(const std::string& componentType)
    {
        if (registry.find(componentType) != registry.end())
        {
            return registry[componentType]();
        }
        return nullptr;
    }

  private:
    std::unordered_map<std::string, RendererCreator> registry;
};

// ============================================================================
// PhysicsModule Component Renderers
// ============================================================================

class PhysicsRigidBodyRenderer : public IComponentRenderer
{
public:
    PhysicsRigidBodyRenderer() : IComponentRenderer() {}

    void render(flecs::entity& entity) override
    {
        ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

        if (ImGui::BeginChildFrame(10, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5f,
                                               ImGui::GetWindowSize().y / 3.f)))
        {
            if (PhysicsModule::RigidBodyComponent* rb = entity.get_mut<PhysicsModule::RigidBodyComponent>())
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.3f, 0.5f));
                bool headerOpen = ImGui::CollapsingHeader("Rigid Body Component", ImGuiTreeNodeFlags_DefaultOpen);
                renderComponentControls(entity, "RigidBodyComponent", "Rigid Body Component");
                
                if (headerOpen)
                {
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosX(0);
                    ImGui::Separator();

                    float mass = rb->mass;
                    bool isStatic = rb->isStatic;
                    bool isKinematic = rb->isKinematic;

                    ImGui::Text("Mass:");
                    ImGui::SameLine();
                    ImGui::BeginDisabled(isStatic || isKinematic);
                    if (ImGui::DragFloat("##Mass", &mass, 0.1f, 0.0f, 1000000.0f))
                    {
                        rb->mass = mass;
                        entity.modified<PhysicsModule::RigidBodyComponent>();
                    }
                    ImGui::EndDisabled();

                    ImGui::Text("Is Static:");
                    ImGui::SameLine();
                    if (ImGui::Checkbox("##IsStatic", &isStatic))
                    {
                        rb->isStatic = isStatic;
                        if (isStatic)
                            rb->isKinematic = false;
                        rb->needsCreation = true;
                        entity.modified<PhysicsModule::RigidBodyComponent>();
                    }

                    ImGui::Text("Is Kinematic:");
                    ImGui::SameLine();
                    ImGui::BeginDisabled(isStatic);
                    if (ImGui::Checkbox("##IsKinematic", &isKinematic))
                    {
                        rb->isKinematic = isKinematic;
                        if (isKinematic)
                            rb->isStatic = false;
                        rb->needsCreation = true;
                        entity.modified<PhysicsModule::RigidBodyComponent>();
                    }
                    ImGui::EndDisabled();
                }
                else
                {
                    ImGui::PopStyleColor();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class PhysicsColliderRenderer : public IComponentRenderer
{
public:
    PhysicsColliderRenderer() : IComponentRenderer() {}

    void render(flecs::entity& entity) override
    {
        ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

        if (ImGui::BeginChildFrame(11, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5f,
                                               ImGui::GetWindowSize().y / 3.f)))
        {
            if (PhysicsModule::ColliderComponent* collider = entity.get_mut<PhysicsModule::ColliderComponent>())
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.3f, 0.5f));
                bool headerOpen = ImGui::CollapsingHeader("Collider Component", ImGuiTreeNodeFlags_DefaultOpen);
                renderComponentControls(entity, "ColliderComponent", "Collider Component");
                
                if (headerOpen)
                {
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosX(0);
                    ImGui::Separator();

                    // Shape Type
                    const char* shapeTypes[] = { "Box", "Sphere", "Capsule", "Cylinder", "Mesh", "ConvexHull" };
                    int currentShapeType = static_cast<int>(collider->shapeDesc.type);
                    
                    ImGui::Text("Shape Type:");
                    ImGui::SameLine();
                    if (ImGui::Combo("##ShapeType", &currentShapeType, shapeTypes, IM_ARRAYSIZE(shapeTypes)))
                    {
                        collider->shapeDesc.type = static_cast<PhysicsModule::PhysicsShapeType>(currentShapeType);
                        collider->needsCreation = true;
                        entity.modified<PhysicsModule::ColliderComponent>();
                    }

                    // Shape parameters based on type
                    if (collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Box ||
                        collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Cylinder)
                    {
                        float size[3] = { collider->shapeDesc.size.x, collider->shapeDesc.size.y, collider->shapeDesc.size.z };
                        ImGui::Text("Size:");
                        ImGui::SameLine();
                        if (ImGui::DragFloat3("##Size", size, 0.01f, 0.01f, 1000.0f))
                        {
                            collider->shapeDesc.size = glm::vec3(size[0], size[1], size[2]);
                            collider->needsCreation = true;
                            entity.modified<PhysicsModule::ColliderComponent>();
                        }
                    }

                    if (collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Sphere ||
                        collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Capsule)
                    {
                        float radius = collider->shapeDesc.radius;
                        ImGui::Text("Radius:");
                        ImGui::SameLine();
                        if (ImGui::DragFloat("##Radius", &radius, 0.01f, 0.01f, 1000.0f))
                        {
                            collider->shapeDesc.radius = radius;
                            collider->needsCreation = true;
                            entity.modified<PhysicsModule::ColliderComponent>();
                        }
                    }

                    if (collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Capsule ||
                        collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Cylinder)
                    {
                        float height = collider->shapeDesc.height;
                        ImGui::Text("Height:");
                        ImGui::SameLine();
                        if (ImGui::DragFloat("##Height", &height, 0.01f, 0.01f, 1000.0f))
                        {
                            collider->shapeDesc.height = height;
                            collider->needsCreation = true;
                            entity.modified<PhysicsModule::ColliderComponent>();
                        }
                    }

                    // Is Trigger
                    bool isTrigger = collider->isTrigger;
                    ImGui::Text("Is Trigger:");
                    ImGui::SameLine();
                    if (ImGui::Checkbox("##IsTrigger", &isTrigger))
                    {
                        collider->isTrigger = isTrigger;
                        collider->needsCreation = true;
                        entity.modified<PhysicsModule::ColliderComponent>();
                    }
                }
                else
                {
                    ImGui::PopStyleColor();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class PhysicsCharacterControllerRenderer : public IComponentRenderer
{
public:
    PhysicsCharacterControllerRenderer() : IComponentRenderer() {}

    void render(flecs::entity& entity) override
    {
        ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

        if (ImGui::BeginChildFrame(12, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5f,
                                               ImGui::GetWindowSize().y / 3.f)))
        {
            if (PhysicsModule::CharacterControllerComponent* cc = entity.get_mut<PhysicsModule::CharacterControllerComponent>())
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.3f, 0.5f));
                bool headerOpen = ImGui::CollapsingHeader("Character Controller Component", ImGuiTreeNodeFlags_DefaultOpen);
                renderComponentControls(entity, "CharacterControllerComponent", "Character Controller Component");
                
                if (headerOpen)
                {
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosX(0);
                    ImGui::Separator();

                    float radius = cc->radius;
                    float height = cc->height;
                    float stepHeight = cc->stepHeight;
                    float velocity[3] = { cc->velocity.x, cc->velocity.y, cc->velocity.z };

                    ImGui::Text("Radius:");
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##Radius", &radius, 0.01f, 0.01f, 10.0f))
                    {
                        cc->radius = radius;
                        entity.modified<PhysicsModule::CharacterControllerComponent>();
                    }

                    ImGui::Text("Height:");
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##Height", &height, 0.01f, 0.01f, 10.0f))
                    {
                        cc->height = height;
                        entity.modified<PhysicsModule::CharacterControllerComponent>();
                    }

                    ImGui::Text("Step Height:");
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##StepHeight", &stepHeight, 0.01f, 0.0f, 5.0f))
                    {
                        cc->stepHeight = stepHeight;
                        entity.modified<PhysicsModule::CharacterControllerComponent>();
                    }

                    ImGui::Text("Velocity:");
                    ImGui::SameLine();
                    if (ImGui::DragFloat3("##Velocity", velocity, 0.1f, -1000.0f, 1000.0f))
                    {
                        cc->velocity = glm::vec3(velocity[0], velocity[1], velocity[2]);
                        entity.modified<PhysicsModule::CharacterControllerComponent>();
                    }

                    ImGui::Text("Is Grounded:");
                    ImGui::SameLine();
                    ImGui::BeginDisabled(true);
                    ImGui::Checkbox("##IsGrounded", &cc->isGrounded);
                    ImGui::EndDisabled();
                }
                else
                {
                    ImGui::PopStyleColor();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};

class PhysicsMaterialRenderer : public IComponentRenderer
{
public:
    PhysicsMaterialRenderer() : IComponentRenderer() {}

    void render(flecs::entity& entity) override
    {
        ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

        if (ImGui::BeginChildFrame(13, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5f,
                                               ImGui::GetWindowSize().y / 3.f)))
        {
            if (PhysicsModule::PhysicsMaterialComponent* material = entity.get_mut<PhysicsModule::PhysicsMaterialComponent>())
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.3f, 0.5f));
                bool headerOpen = ImGui::CollapsingHeader("Physics Material Component", ImGuiTreeNodeFlags_DefaultOpen);
                renderComponentControls(entity, "PhysicsMaterialComponent", "Physics Material Component");
                
                if (headerOpen)
                {
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosX(0);
                    ImGui::Separator();

                    float friction = material->friction;
                    float restitution = material->restitution;
                    float density = material->density;

                    ImGui::Text("Friction:");
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##Friction", &friction, 0.01f, 0.0f, 10.0f))
                    {
                        material->friction = friction;
                        entity.modified<PhysicsModule::PhysicsMaterialComponent>();
                    }

                    ImGui::Text("Restitution:");
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##Restitution", &restitution, 0.01f, 0.0f, 1.0f))
                    {
                        material->restitution = restitution;
                        entity.modified<PhysicsModule::PhysicsMaterialComponent>();
                    }

                    ImGui::Text("Density:");
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##Density", &density, 0.01f, 0.01f, 10000.0f))
                    {
                        material->density = density;
                        entity.modified<PhysicsModule::PhysicsMaterialComponent>();
                    }
                }
                else
                {
                    ImGui::PopStyleColor();
                }
            }
        }
        ImGui::EndChildFrame();
    }
};
