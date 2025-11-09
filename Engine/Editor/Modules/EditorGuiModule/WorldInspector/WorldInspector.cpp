#include "WorldInspector.h"
#include "../Events.h"
#include <imgui.h>
#include <filesystem>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ObjectCoreModule/ECS/ComponentRegistry.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/ObjectCoreModule/ECS/Systems/ECSLuaScriptsSystem.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ObjectCoreModule/ECS/Systems/ECSPhysicsSystem.h>
#include "ComponentsRenderFabric.h"
#include <Core/CoreGlobal.h>

flecs::entity GUIWorldInspector::m_selectedEntity = {};

GUIWorldInspector::GUIWorldInspector() :
    GUIObject(),m_ecsModule(GCM(ECSModule::ECSModule)),
    m_world(m_ecsModule->getCurrentWorld()->get())
{
    ComponentRendererFactory& factory = ComponentRendererFactory::getInstance();
    factory.registerRenderer("PositionComponent", [this]()
    {
        return std::make_unique<PositionRenderer>();
    });
    factory.registerRenderer("RotationComponent", [this]()
    {
        return std::make_unique<RotationRenderer>();
    });
    factory.registerRenderer("ScaleComponent", [this]()
    {
        return std::make_unique<ScaleRenderer>();
    });
    factory.registerRenderer("ScriptComponent", [this]()
    {
        return std::make_unique<ScriptRenderer>();
    });
    factory.registerRenderer("MeshComponent", [this]()
    {
        return std::make_unique<MeshComponentRenderer>();
    });
    factory.registerRenderer("CameraComponent", [this]()
    {
        return std::make_unique<CameraRenderer>();
    });
    factory.registerRenderer("DirectionalLightComponent", [this]()
    {
        return std::make_unique<DirectionalLightRenderer>();
    });

    factory.registerRenderer("PointLightComponent", [this]()
    {
        return std::make_unique<PointLightRenderer>();
    });
    factory.registerRenderer("RigidbodyComponent", [this]()
    {
        return std::make_unique<RigidbodyRenderer>();
    });
    factory.registerRenderer("MaterialComponent", [this]()
    {
        return std::make_unique<MaterialRenderer>();
    });
}

void GUIWorldInspector::render(float deltaTime)
{
    // Handle ESC key to deselect entity
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        if (m_selectedEntity.is_valid())
        {
            Events::EditorUI::EntityDeselected evt{};
            GCEB().emit(evt);
            m_selectedEntity = {};
        }
    }

    if (!isVisible())
        return;

    bool windowOpen = true;
    if (ImGui::Begin("WorldInspector", &windowOpen, 0))
    {
        ImGui::BeginChild("WorldTree", ImVec2(0, ImGui::GetWindowHeight() * 0.3f), true);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Tree").x) / 2);

        ImGui::SetWindowFontScale(1.2f);
        ImGui::Text("Tree");
        ImGui::SetWindowFontScale(1);

        ImGui::SetCursorPosX(0);
        ImGui::Separator();
        renderEntityTree();
        renderEntityTreePopup();
        ImGui::EndChild();
        ImGui::BeginChild("ObjectDefault#", ImVec2(0, 0), true);
        renderSelectedEntityDefaults();
           ImGui::EndChild();
    }
    
    // Handle window close button
    if (!windowOpen)
    {
        hide();
    }
    
    ImGui::End();
}

void GUIWorldInspector::renderEntityTreePopup()
{
    if (ImGui::BeginPopupContextWindow())
    {
        if (ImGui::Button("Add entity"))
        {
            ImGui::OpenPopup("CreateEntityPopup##2");
        }
        if (ImGui::BeginPopup("CreateEntityPopup##2"))
        {
            static char buffer[128] = "";
            ImGui::InputText("##EntityName", buffer, sizeof(buffer));

            ImGui::SameLine();
            std::string strBuffer = buffer;
            if (ImGui::Button("CreateEntity##5"))
            {
                if (strBuffer.size() > 0)
                {
                    Events::EditorUI::EntityCreateRequest evt{};
                    evt.entityName = buffer;
                    evt.withDefaultComponents = true;
                    GCEB().emit(evt);
                }
            }
            ImGui::EndPopup();
        }
        ImGui::EndPopup();
    }
}

void GUIWorldInspector::renderEntityTree()
{
    auto query = m_world.query<PositionComponent>();
    query.each([&](flecs::entity e, PositionComponent pos)
    {
        if (!e.has<InvisibleTag>())
        {
            bool isSelected = m_selectedEntity.is_valid() && m_selectedEntity == e;
            if (ImGui::Selectable(std::format("{}##{}", e.name().c_str(), e.id()).c_str(), isSelected))
            {
                m_selectedEntity = e;
                Events::EditorUI::EntitySelected evt{};
                evt.entityId = e.id();
                GCEB().emit(evt);
            }
        }
    });
}

void GUIWorldInspector::renderSelectedEntityDefaults()
{
    if (m_selectedEntity.is_valid())
    {
        // Entity name header
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(m_selectedEntity.name()).x) / 2);
        ImGui::SetWindowFontScale(1.2f);
        ImGui::TextColored(ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f),
                           m_selectedEntity.name());
        ImGui::SetWindowFontScale(1);
        ImGui::SetCursorPosX(0);
        ImGui::Separator();
        ImGui::Spacing();

        auto& factory = ComponentRendererFactory::getInstance();
        auto& registry = ECSModule::ComponentRegistry::getInstance();
        
        // Dynamically render all components that entity has
        auto availableComponents = registry.getAvailableComponents();
        for (const auto& [typeName, displayName] : availableComponents)
        {
            if (registry.hasComponent(m_selectedEntity, typeName))
            {
                if (auto renderer = factory.createRenderer(typeName))
                {
                    renderer->render(m_selectedEntity, typeName, displayName);
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Action buttons
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("+ Add Component     Delete Entity").x) / 2);

        if (ImGui::Button("+ Add Component"))
        {
            ImGui::OpenPopup("AddComponent");
        }

        renderAddComponentPopup();

        ImGui::SameLine();

        if (ImGui::Button("Delete Entity"))
        {
            Events::EditorUI::EntityDeleteRequest evt{};
            evt.entityId = m_selectedEntity.id();
            GCEB().emit(evt);
            m_selectedEntity = {};
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No entity selected");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select an entity from the tree above");
    }
}

void GUIWorldInspector::renderAddComponentPopup()
{
    if (!m_selectedEntity.is_valid())
        return;

    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x / 3, ImGui::GetIO().DisplaySize.y / 2));
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), 0,
                            ImVec2(0.5f, 0.5f));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    if (ImGui::BeginPopupModal("AddComponent", nullptr, flags))
    {
        ImGui::Text("Select component to add:");
        ImGui::Separator();

        static int selectedIndex = -1;
        static std::string preview = "Select component...";

        if (ImGui::BeginCombo("##ComponentList", preview.c_str()))
        {
            auto& registry = ECSModule::ComponentRegistry::getInstance();
            auto availableComponents = registry.getAvailableComponents();
            
            for (size_t i = 0; i < availableComponents.size(); ++i)
            {
                const auto& [typeName, displayName] = availableComponents[i];
                
                // Check if entity already has this component using registry
                bool hasComponent = registry.hasComponent(m_selectedEntity, typeName);

                if (hasComponent)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                    ImGui::Text("%s (already added)", displayName.c_str());
                    ImGui::PopStyleColor();
                }
                else
                {
                    bool isSelected = (selectedIndex == static_cast<int>(i));
                    if (ImGui::Selectable(displayName.c_str(), isSelected))
                    {
                        selectedIndex = static_cast<int>(i);
                        preview = displayName;
                        
                        // Emit event to add component
                        Events::EditorUI::ComponentAddRequest evt{};
                        evt.entityId = m_selectedEntity.id();
                        evt.componentTypeName = typeName;
                        GCEB().emit(evt);
                        
                        ImGui::CloseCurrentPopup();
                        selectedIndex = -1;
                        preview = "Select component...";
                    }
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
            selectedIndex = -1;
            preview = "Select component...";
        }

        ImGui::EndPopup();
    }
}
