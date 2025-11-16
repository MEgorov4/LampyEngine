#pragma once

#include <EngineMinimal.h>
#include <algorithm>
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
    
    std::string getAssetDisplayName(const ResourceModule::AssetID& assetID) const
    {
        if (assetID.empty())
            return "None";
        
        if (auto* db = ResourceModule::AssetRegistryAccessor::Get())
        {
            if (auto infoOpt = db->get(assetID))
            {
                std::filesystem::path path(infoOpt->sourcePath);
                return path.filename().string();
            }
        }
        
        return assetID.str();
    }

    bool beginComponentPanel(flecs::entity& entity, const std::string& typeName, const std::string& displayName) const
    {
        ImGui::PushID(typeName.c_str());
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 2.0f));
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.19f, 0.24f, 0.9f));

        bool open = ImGui::CollapsingHeader(displayName.c_str(),
                                            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
        renderComponentControls(entity, typeName);
        ImGui::PopStyleColor();

        if (!open)
        {
            ImGui::PopStyleVar(2);
            ImGui::PopID();
        }

        return open;
    }

    void endComponentPanel() const
    {
        ImGui::PopStyleVar(2);
        ImGui::PopID();
        ImGui::Spacing();
    }

    bool beginPropertyTable() const
    {
        if (ImGui::BeginTable("Properties", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            return true;
        }
        return false;
    }

    void endPropertyTable() const
    {
        ImGui::EndTable();
    }

  public:
    IComponentRenderer() :
        m_projectModule(GCXM(ProjectModule::ProjectModule))
    {
    }

    virtual void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName)
    {
        render(entity);
    }

    virtual void render(flecs::entity& entity) = 0;

    void renderComponentControls(flecs::entity& entity, const std::string& typeName) const
    {
        ImGui::PushID((typeName + "_controls").c_str());

        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        const float buttonWidthReset = ImGui::CalcTextSize("↻ Reset").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        const float buttonWidthRemove = ImGui::CalcTextSize("× Remove").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        const float totalWidth = buttonWidthReset + buttonWidthRemove + spacing;

        float cursorStart = ImGui::GetCursorPosX();
        float available = ImGui::GetContentRegionAvail().x;
        float newX = cursorStart + std::max(0.0f, available - totalWidth);
        ImGui::SameLine(newX);

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

        bool isCoreTransform = (typeName == "TransformComponent");

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
        else if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Remove component from entity");
        }

        ImGui::PopID();
    }

    virtual ~IComponentRenderer() = default;
};

class TransformRenderer : public IComponentRenderer
{
  public:
    TransformRenderer() : IComponentRenderer()
    {
    }

    void render(flecs::entity& entity) override
    {
        render(entity, "TransformComponent", "Transform");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (const TransformComponent* transform = entity.get<TransformComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            if (beginPropertyTable())
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Position");
                ImGui::TableSetColumnIndex(1);
                float position[3] = {transform->position.x, transform->position.y, transform->position.z};
                if (ImGui::DragFloat3("##Position", position, 0.01f))
                {
                    auto& mutableTransform = EnsureTransformComponent(entity);
                    mutableTransform.position = {position[0], position[1], position[2]};
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Rotation");
                ImGui::TableSetColumnIndex(1);
                float rotation[3] = {transform->rotation.x, transform->rotation.y, transform->rotation.z};
                if (ImGui::DragFloat3("##Rotation", rotation, 1.0f))
                {
                    auto& mutableTransform = EnsureTransformComponent(entity);
                    RotationComponent newRot;
                    newRot.fromEulerDegrees(glm::vec3(rotation[0], rotation[1], rotation[2]));
                    mutableTransform.rotation = newRot;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Scale");
                ImGui::TableSetColumnIndex(1);
                float scale[3] = {transform->scale.x, transform->scale.y, transform->scale.z};
                if (ImGui::DragFloat3("##Scale", scale, 0.01f, -100.0f, 100.0f))
                {
                    auto& mutableTransform = EnsureTransformComponent(entity);
                    mutableTransform.scale = {scale[0], scale[1], scale[2]};
                }
                endPropertyTable();
            }

            endComponentPanel();
        }
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
        render(entity, "MeshComponent", "Mesh");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (const MeshComponent* meshComponent = entity.get<MeshComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            std::string resPath = m_projectModule->getProjectConfig().getResourcesPath();

            if (beginPropertyTable())
            {
                auto drawAssetField = [&](const char* label,
                                          const ResourceModule::AssetID& value,
                                          auto onAssetDrop,
                                          auto onFileDrop)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(label);
                    ImGui::TableSetColumnIndex(1);
                    std::string display = getAssetDisplayName(value);
                    if (value.empty())
                        display = std::string("None (drop ") + label + ")";
                    ImVec4 color = value.empty() ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f) : ImVec4(0.8f, 0.8f, 1.0f, 1.0f);
                    ImGui::TextColored(color, "%s", display.c_str());

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetID"))
                        {
                            std::string assetIDStr = static_cast<const char*>(payload->Data);
                            ResourceModule::AssetID assetID(assetIDStr);
                            onAssetDrop(assetID);
                        }
                        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath"))
                        {
                            std::string droppedPath = static_cast<const char*>(payload->Data);
                            std::filesystem::path absPath(droppedPath);
                            std::filesystem::path resRoot(resPath);
                            std::filesystem::path relativePath;

                            try
                            {
                                if (std::filesystem::exists(absPath))
                                {
                                    auto canonicalPath = std::filesystem::canonical(absPath);
                                    if (canonicalPath.string().rfind(resRoot.string(), 0) == 0)
                                    {
                                        relativePath = std::filesystem::relative(canonicalPath, resRoot);
                                    }
                                    else
                                    {
                                        relativePath = canonicalPath;
                                    }
                                }
                                else
                                {
                                    relativePath = absPath;
                                }
                            }
                            catch (...)
                            {
                                relativePath = absPath;
                            }

                            onFileDrop(relativePath.generic_string());
                        }
                        ImGui::EndDragDropTarget();
                    }
                };

                drawAssetField(
                    "Mesh",
                    meshComponent->meshID,
                    [&](const ResourceModule::AssetID& assetID)
                    {
                        if (auto mesh = entity.get_mut<MeshComponent>())
                        {
                            mesh->meshID = assetID;
                            entity.modified<MeshComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                        }
                    },
                    [&](const std::string& path)
                    {
                        std::filesystem::path p(path);
                        std::string ext = p.extension().string();
                        if (ext == ".obj" || ext == ".meshbin")
                        {
                            if (auto mesh = entity.get_mut<MeshComponent>())
                            {
                                mesh->meshID = ResourceModule::AssetID(std::filesystem::path(path).generic_string());
                                entity.modified<MeshComponent>();
                                GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                            }
                        }
                    });

                drawAssetField(
                    "Texture",
                    meshComponent->textureID,
                    [&](const ResourceModule::AssetID& assetID)
                    {
                        if (auto mesh = entity.get_mut<MeshComponent>())
                        {
                            mesh->textureID = assetID;
                            entity.modified<MeshComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                        }
                    },
                    [&](const std::string& path)
                    {
                        std::string ext = std::filesystem::path(path).extension().string();
                        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                        {
                            if (auto mesh = entity.get_mut<MeshComponent>())
                            {
                                mesh->textureID = ResourceModule::AssetID(std::filesystem::path(path).generic_string());
                                entity.modified<MeshComponent>();
                                GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                            }
                        }
                    });

                drawAssetField(
                    "Vertex Shader",
                    meshComponent->vertShaderID,
                    [&](const ResourceModule::AssetID& assetID)
                    {
                        if (auto mesh = entity.get_mut<MeshComponent>())
                        {
                            mesh->vertShaderID = assetID;
                            entity.modified<MeshComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                        }
                    },
                    [&](const std::string& path)
                    {
                        if (std::filesystem::path(path).extension() == ".vert")
                        {
                            if (auto mesh = entity.get_mut<MeshComponent>())
                            {
                                mesh->vertShaderID = ResourceModule::AssetID(std::filesystem::path(path).generic_string());
                                entity.modified<MeshComponent>();
                                GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                            }
                        }
                    });

                drawAssetField(
                    "Fragment Shader",
                    meshComponent->fragShaderID,
                    [&](const ResourceModule::AssetID& assetID)
                    {
                        if (auto mesh = entity.get_mut<MeshComponent>())
                        {
                            mesh->fragShaderID = assetID;
                            entity.modified<MeshComponent>();
                            GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                        }
                    },
                    [&](const std::string& path)
                    {
                        if (std::filesystem::path(path).extension() == ".frag")
                        {
                            if (auto mesh = entity.get_mut<MeshComponent>())
                            {
                                mesh->fragShaderID = ResourceModule::AssetID(std::filesystem::path(path).generic_string());
                                entity.modified<MeshComponent>();
                                GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MeshComponent"});
                            }
                        }
                    });

                endPropertyTable();
            }

            endComponentPanel();
        }
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
        render(entity, "ScriptComponent", "Script");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (const ScriptComponent* script = entity.get<ScriptComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            auto assignScript = [&](const ResourceModule::AssetID& assetID)
            {
                if (ScriptComponent* scriptMut = entity.get_mut<ScriptComponent>())
                {
                    if (scriptMut->scriptID == assetID)
                        return;
                    scriptMut->scriptID = assetID;
                    entity.modified<ScriptComponent>();
                }
                else
                {
                    ScriptComponent newComponent{};
                    newComponent.scriptID = assetID;
                    entity.set<ScriptComponent>(newComponent);
                }
                GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "ScriptComponent"});
            };

            if (beginPropertyTable())
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Script");
                ImGui::TableSetColumnIndex(1);
                std::string displayName = getAssetDisplayName(script->scriptID);
                if (script->scriptID.empty())
                    displayName = "None (drop .lua asset)";
                ImVec4 color = script->scriptID.empty()
                                   ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f)
                                   : ImVec4(0.8f, 0.8f, 1.0f, 1.0f);
                ImGui::TextColored(color, "%s", displayName.c_str());

                if (!script->scriptID.empty())
                {
                    ImGui::SameLine();
                    ImGui::TextDisabled("%s", script->scriptID.str().c_str());
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetID"))
                    {
                        std::string assetIDStr = static_cast<const char*>(payload->Data);
                        ResourceModule::AssetID assetID(assetIDStr);
                        bool accept = true;
                        if (auto* db = ResourceModule::AssetRegistryAccessor::Get())
                        {
                            if (auto infoOpt = db->get(assetID))
                            {
                                accept = (infoOpt->type == ResourceModule::AssetType::Script);
                            }
                        }
                        if (accept)
                        {
                            assignScript(assetID);
                        }
                    }
                    else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath"))
                    {
                        std::string droppedPath = static_cast<const char*>(payload->Data);
                        std::filesystem::path absPath(droppedPath);
                        if (absPath.extension() == ".lua")
                        {
                            std::filesystem::path resRoot(m_projectModule->getProjectConfig().getResourcesPath());
                            std::filesystem::path relativePath;
                            try
                            {
                                if (std::filesystem::exists(absPath))
                                {
                                    auto canonicalPath = std::filesystem::canonical(absPath);
                                    if (canonicalPath.string().rfind(resRoot.string(), 0) == 0)
                                    {
                                        relativePath = std::filesystem::relative(canonicalPath, resRoot);
                                    }
                                    else
                                    {
                                        relativePath = canonicalPath;
                                    }
                                }
                                else
                                {
                                    relativePath = absPath;
                                }
                            }
                            catch (...)
                            {
                                relativePath = absPath;
                            }

                            if (!relativePath.empty())
                            {
                                assignScript(ResourceModule::AssetID(relativePath.generic_string()));
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Actions");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::Button("Clear Script"))
                {
                    assignScript(ResourceModule::AssetID());
                }

                endPropertyTable();
            }

            endComponentPanel();
        }
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
        render(entity, "DirectionalLightComponent", "Directional Light");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (const DirectionalLightComponent* dirLightComponent = entity.get<DirectionalLightComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            if (beginPropertyTable())
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Intensity");
                ImGui::TableSetColumnIndex(1);
                float intensity = dirLightComponent->intencity;
                if (ImGui::DragFloat("##DirIntensity", &intensity, 0.01f, 0.0f, 1000.0f))
                {
                    entity.set<DirectionalLightComponent>({intensity});
                    GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "DirectionalLightComponent"});
                }

                endPropertyTable();
            }

            endComponentPanel();
        }
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
        render(entity, "PointLightComponent", "Point Light");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (const PointLightComponent* pointLightComponent = entity.get<PointLightComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            if (beginPropertyTable())
            {
                float color[3] = {pointLightComponent->color.x, pointLightComponent->color.y, pointLightComponent->color.z};
                float intensity = pointLightComponent->intencity;
                float innerRadius = pointLightComponent->innerRadius;
                float outerRadius = pointLightComponent->outerRadius;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Color");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::ColorEdit3("##PointColor", color, ImGuiColorEditFlags_NoInputs))
                {
                    if (auto comp = entity.get_mut<PointLightComponent>())
                    {
                        comp->color = glm::vec3(color[0], color[1], color[2]);
                        entity.modified<PointLightComponent>();
                        GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "PointLightComponent"});
                    }
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Intensity");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::DragFloat("##PointIntensity", &intensity, 0.01f, 0.0f, 1000.0f))
                {
                    if (auto comp = entity.get_mut<PointLightComponent>())
                    {
                        comp->intencity = intensity;
                        entity.modified<PointLightComponent>();
                        GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "PointLightComponent"});
                    }
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Inner Radius");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::DragFloat("##PointInnerRadius", &innerRadius, 0.1f, 0.0f, 1000.0f))
                {
                    if (auto comp = entity.get_mut<PointLightComponent>())
                    {
                        comp->innerRadius = std::min(innerRadius, comp->outerRadius);
                        entity.modified<PointLightComponent>();
                        GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "PointLightComponent"});
                    }
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Outer Radius");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::DragFloat("##PointOuterRadius", &outerRadius, 0.1f, 0.0f, 1000.0f))
                {
                    if (auto comp = entity.get_mut<PointLightComponent>())
                    {
                        comp->outerRadius = std::max(outerRadius, comp->innerRadius);
                        entity.modified<PointLightComponent>();
                        GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "PointLightComponent"});
                    }
                }

                endPropertyTable();
            }

            endComponentPanel();
        }
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
        render(entity, "CameraComponent", "Camera");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (CameraComponent* camera = entity.get_mut<CameraComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            if (beginPropertyTable())
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Field of View");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::SliderFloat("##CameraFOV", &camera->fov, 30.0f, 180.0f))
                {
                    entity.modified<CameraComponent>();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Aspect Ratio");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::SliderFloat("##CameraAspect", &camera->aspect, 0.01f, 1.0f))
                {
                    entity.modified<CameraComponent>();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Far Clip");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::SliderFloat("##CameraFar", &camera->farClip, 10.0f, 10000.0f))
                {
                    entity.modified<CameraComponent>();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Near Clip");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::SliderFloat("##CameraNear", &camera->nearClip, 0.01f, 10.0f))
                {
                    entity.modified<CameraComponent>();
                }

                endPropertyTable();
            }

            endComponentPanel();
        }
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
        render(entity, "RigidbodyComponent", "Rigidbody");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (RigidbodyComponent* body = entity.get_mut<RigidbodyComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            if (beginPropertyTable())
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Static");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::Checkbox("##RigidStatic", &body->isStatic))
                {
                    entity.modified<RigidbodyComponent>();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Mass");
                ImGui::TableSetColumnIndex(1);
                ImGui::BeginDisabled(body->isStatic);
                if (ImGui::DragFloat("##RigidMass", &body->mass, 0.1f, 0.0f, 100000000.0f))
                {
                    entity.modified<RigidbodyComponent>();
                }
                ImGui::EndDisabled();

                endPropertyTable();
            }

            endComponentPanel();
        }
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
        render(entity, "MaterialComponent", "Material");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (const MaterialComponent* materialComponent = entity.get<MaterialComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            if (beginPropertyTable())
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Material");
                ImGui::TableSetColumnIndex(1);

                std::string display = getAssetDisplayName(materialComponent->materialID);
                if (materialComponent->materialID.empty())
                    display = "None (drop material asset)";
                ImVec4 color = materialComponent->materialID.empty()
                                   ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f)
                                   : ImVec4(0.8f, 0.8f, 1.0f, 1.0f);
                ImGui::TextColored(color, "%s", display.c_str());

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetID"))
                    {
                        std::string assetIDStr = static_cast<const char*>(payload->Data);
                        ResourceModule::AssetID assetID(assetIDStr);
                        if (auto* db = ResourceModule::AssetRegistryAccessor::Get())
                        {
                            if (auto infoOpt = db->get(assetID))
                            {
                                if (infoOpt->type == ResourceModule::AssetType::Material)
                                {
                                    if (auto mat = entity.get_mut<MaterialComponent>())
                                    {
                                        mat->materialID = assetID;
                                        entity.modified<MaterialComponent>();
                                        GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MaterialComponent"});
                                    }
                                }
                            }
                        }
                    }
                    else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath"))
                    {
                        std::string droppedPath = static_cast<const char*>(payload->Data);
                        std::filesystem::path absPath(droppedPath);
                        if (absPath.extension() == ".lmat")
                        {
                            std::filesystem::path relativePath;
                            std::string resPath = m_projectModule->getProjectConfig().getResourcesPath();
                            try
                            {
                                if (std::filesystem::exists(absPath))
                                {
                                    auto canonicalPath = std::filesystem::canonical(absPath);
                                    std::filesystem::path resRoot(resPath);
                                    if (canonicalPath.string().rfind(resRoot.string(), 0) == 0)
                                    {
                                        relativePath = std::filesystem::relative(canonicalPath, resRoot);
                                    }
                                    else
                                    {
                                        relativePath = canonicalPath;
                                    }
                                }
                                else
                                {
                                    relativePath = absPath;
                                }
                            }
                            catch (...)
                            {
                                relativePath = absPath;
                            }

                            if (auto mat = entity.get_mut<MaterialComponent>())
                            {
                                mat->materialID =
                                    ResourceModule::AssetID(relativePath.generic_string());
                                entity.modified<MaterialComponent>();
                                GCEB().emit(Events::ECS::ComponentChanged{entity.id(), "MaterialComponent"});
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                endPropertyTable();
            }

            if (!materialComponent->materialID.empty())
            {
                if (auto rm = GCM(ResourceModule::ResourceManager))
                {
                    auto material = rm->load<ResourceModule::RMaterial>(materialComponent->materialID);
                    if (material)
                    {
                        ImGui::Separator();
                        ImGui::Text("Name: %s", material->name.c_str());
                        float albedo[4] = {material->albedoColor.r,
                                           material->albedoColor.g,
                                           material->albedoColor.b,
                                           material->albedoColor.a};
                        ImGui::TextUnformatted("Albedo Color");
                        ImGui::SameLine();
                        ImGui::ColorEdit4("##AlbedoPreview", albedo,
                                          ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                        ImGui::Text("Roughness: %.2f", material->roughness);
                        ImGui::Text("Metallic: %.2f", material->metallic);
                    }
                }
            }

            endComponentPanel();
        }
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
        render(entity, "RigidBodyComponent", "Rigid Body");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (auto rb = entity.get_mut<PhysicsModule::RigidBodyComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            if (beginPropertyTable())
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Mass");
                ImGui::TableSetColumnIndex(1);
                ImGui::BeginDisabled(rb->isStatic || rb->isKinematic);
                if (ImGui::DragFloat("##PhysMass", &rb->mass, 0.1f, 0.0f, 1000000.0f))
                {
                    entity.modified<PhysicsModule::RigidBodyComponent>();
                }
                ImGui::EndDisabled();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Static");
                ImGui::TableSetColumnIndex(1);
                bool isStatic = rb->isStatic;
                if (ImGui::Checkbox("##PhysStatic", &isStatic))
                {
                    rb->isStatic = isStatic;
                    if (isStatic)
                        rb->isKinematic = false;
                    rb->needsCreation = true;
                    entity.modified<PhysicsModule::RigidBodyComponent>();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Kinematic");
                ImGui::TableSetColumnIndex(1);
                bool isKinematic = rb->isKinematic;
                ImGui::BeginDisabled(rb->isStatic);
                if (ImGui::Checkbox("##PhysKinematic", &isKinematic))
                {
                    rb->isKinematic = isKinematic;
                    if (isKinematic)
                        rb->isStatic = false;
                    rb->needsCreation = true;
                    entity.modified<PhysicsModule::RigidBodyComponent>();
                }
                ImGui::EndDisabled();

                endPropertyTable();
            }

            endComponentPanel();
        }
    }
};

class PhysicsColliderRenderer : public IComponentRenderer
{
public:
    PhysicsColliderRenderer() : IComponentRenderer() {}

    void render(flecs::entity& entity) override
    {
        render(entity, "ColliderComponent", "Collider");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (auto collider = entity.get_mut<PhysicsModule::ColliderComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            if (beginPropertyTable())
            {
                const char* shapeTypes[] = {"Box", "Sphere", "Capsule", "Cylinder", "Mesh", "ConvexHull"};
                int currentShapeType = static_cast<int>(collider->shapeDesc.type);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Shape");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::Combo("##ColliderShape", &currentShapeType, shapeTypes, IM_ARRAYSIZE(shapeTypes)))
                {
                    collider->shapeDesc.type = static_cast<PhysicsModule::PhysicsShapeType>(currentShapeType);
                    collider->needsCreation = true;
                    entity.modified<PhysicsModule::ColliderComponent>();
                }

                if (collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Box ||
                    collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Cylinder)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Size");
                    ImGui::TableSetColumnIndex(1);
                    float size[3] = {collider->shapeDesc.size.x, collider->shapeDesc.size.y, collider->shapeDesc.size.z};
                    if (ImGui::DragFloat3("##ColliderSize", size, 0.01f, 0.01f, 1000.0f))
                    {
                        collider->shapeDesc.size = glm::vec3(size[0], size[1], size[2]);
                        collider->needsCreation = true;
                        entity.modified<PhysicsModule::ColliderComponent>();
                    }
                }

                if (collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Sphere ||
                    collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Capsule)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Radius");
                    ImGui::TableSetColumnIndex(1);
                    if (ImGui::DragFloat("##ColliderRadius", &collider->shapeDesc.radius, 0.01f, 0.01f, 1000.0f))
                    {
                        collider->needsCreation = true;
                        entity.modified<PhysicsModule::ColliderComponent>();
                    }
                }

                if (collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Capsule ||
                    collider->shapeDesc.type == PhysicsModule::PhysicsShapeType::Cylinder)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Height");
                    ImGui::TableSetColumnIndex(1);
                    if (ImGui::DragFloat("##ColliderHeight", &collider->shapeDesc.height, 0.01f, 0.01f, 1000.0f))
                    {
                        collider->needsCreation = true;
                        entity.modified<PhysicsModule::ColliderComponent>();
                    }
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Trigger");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::Checkbox("##ColliderTrigger", &collider->isTrigger))
                {
                    collider->needsCreation = true;
                    entity.modified<PhysicsModule::ColliderComponent>();
                }

                endPropertyTable();
            }

            endComponentPanel();
        }
    }
};

class PhysicsCharacterControllerRenderer : public IComponentRenderer
{
public:
    PhysicsCharacterControllerRenderer() : IComponentRenderer() {}

    void render(flecs::entity& entity) override
    {
        render(entity, "CharacterControllerComponent", "Character Controller");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (auto cc = entity.get_mut<PhysicsModule::CharacterControllerComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            if (beginPropertyTable())
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Radius");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::DragFloat("##CCRadius", &cc->radius, 0.01f, 0.01f, 10.0f))
                {
                    entity.modified<PhysicsModule::CharacterControllerComponent>();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Height");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::DragFloat("##CCHeight", &cc->height, 0.01f, 0.01f, 10.0f))
                {
                    entity.modified<PhysicsModule::CharacterControllerComponent>();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Step Height");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::DragFloat("##CCStepHeight", &cc->stepHeight, 0.01f, 0.0f, 5.0f))
                {
                    entity.modified<PhysicsModule::CharacterControllerComponent>();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Velocity");
                ImGui::TableSetColumnIndex(1);
                float velocity[3] = {cc->velocity.x, cc->velocity.y, cc->velocity.z};
                if (ImGui::DragFloat3("##CCVelocity", velocity, 0.1f, -1000.0f, 1000.0f))
                {
                    cc->velocity = glm::vec3(velocity[0], velocity[1], velocity[2]);
                    entity.modified<PhysicsModule::CharacterControllerComponent>();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Grounded");
                ImGui::TableSetColumnIndex(1);
                ImGui::BeginDisabled(true);
                ImGui::Checkbox("##CCGrounded", &cc->isGrounded);
                ImGui::EndDisabled();

                endPropertyTable();
            }

            endComponentPanel();
        }
    }
};

class PhysicsMaterialRenderer : public IComponentRenderer
{
public:
    PhysicsMaterialRenderer() : IComponentRenderer() {}

    void render(flecs::entity& entity) override
    {
        render(entity, "PhysicsMaterialComponent", "Physics Material");
    }

    void render(flecs::entity& entity, const std::string& typeName, const std::string& displayName) override
    {
        if (auto material = entity.get_mut<PhysicsModule::PhysicsMaterialComponent>())
        {
            if (!beginComponentPanel(entity, typeName, displayName))
                return;

            if (beginPropertyTable())
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Friction");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::DragFloat("##PhysFriction", &material->friction, 0.01f, 0.0f, 10.0f))
                {
                    entity.modified<PhysicsModule::PhysicsMaterialComponent>();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Restitution");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::DragFloat("##PhysRestitution", &material->restitution, 0.01f, 0.0f, 1.0f))
                {
                    entity.modified<PhysicsModule::PhysicsMaterialComponent>();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Density");
                ImGui::TableSetColumnIndex(1);
                if (ImGui::DragFloat("##PhysDensity", &material->density, 0.01f, 0.01f, 10000.0f))
                {
                    entity.modified<PhysicsModule::PhysicsMaterialComponent>();
                }

                endPropertyTable();
            }

            endComponentPanel();
        }
    }
};
