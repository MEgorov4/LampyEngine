#include "WorldInspector.h"
#include <imgui.h>
#include <filesystem>
#include "../../ObjectCoreModule/ECS/ECSModule.h"
#include "../../ProjectModule/ProjectModule.h"
#include "../../ObjectCoreModule/ECS/ECSLuaScriptsSystem.h"
#include "ComponentsRenderFabric.h"


GUIWorldInspector::GUIWorldInspector(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                                     const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                                     const std::shared_ptr<ECSModule::ECSModule>& ecsModule) :
    GUIObject(), m_projectModule(projectModule), m_filesystemModule(filesystemModule), m_ecsModule(ecsModule),
    m_world(ecsModule->getCurrentWorld())
{
    ComponentRendererFactory& factory = ComponentRendererFactory::getInstance();
    factory.registerRenderer("PositionComponent", [this]()
    {
        return std::make_unique<PositionRenderer>(m_projectModule, m_filesystemModule);
    });
    factory.registerRenderer("RotationComponent", [this]()
    {
        return std::make_unique<RotationRenderer>(m_projectModule, m_filesystemModule);
    });
    factory.registerRenderer("ScaleComponent", [this]()
    {
        return std::make_unique<ScaleRenderer>(m_projectModule, m_filesystemModule);
    });
    factory.registerRenderer("ScriptComponent", [this]()
    {
        return std::make_unique<ScriptRenderer>(m_projectModule, m_filesystemModule);
    });
    factory.registerRenderer("MeshComponent", [this]()
    {
        return std::make_unique<MeshComponentRenderer>(m_projectModule, m_filesystemModule);
    });
    factory.registerRenderer("CameraComponent", [this]()
    {
        return std::make_unique<CameraRenderer>(m_projectModule, m_filesystemModule);
    });
    factory.registerRenderer("DirectionalLightComponent", [this]()
    {
        return std::make_unique<DirectionalLightRenderer>(m_projectModule, m_filesystemModule);
    });
    factory.registerRenderer("RigidbodyComponent", [this]()
    {
        return std::make_unique<RigidbodyRenderer>(m_projectModule, m_filesystemModule);
    });
}

void GUIWorldInspector::render()
{
    if (ImGui::Begin("WorldInspector", nullptr, 0))
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
                    m_world.entity(buffer).set<PositionComponent>({0, 0, 0})
                           .set<RotationComponent>({0, 0, 0})
                           .set<ScaleComponent>({1, 1, 1});
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
            if (ImGui::Selectable(std::format("{}##{}", e.name().c_str(), e.id()).c_str()))
                m_selectedEntity = e;
    });
}

void GUIWorldInspector::renderSelectedEntityDefaults()
{
    if (m_selectedEntity.is_valid())
    {
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(m_selectedEntity.name()).x) / 2);
        ImGui::SetWindowFontScale(1.2f);
        ImGui::TextColored(ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f),
                           m_selectedEntity.name());
        ImGui::SetWindowFontScale(1);
        ImGui::SetCursorPosX(0);

        auto& factory = ComponentRendererFactory::getInstance();

        if (m_selectedEntity.has<PositionComponent>())
        {
            if (auto renderer = factory.createRenderer("PositionComponent"))
            {
                renderer->render(m_selectedEntity);
            }
        }

        if (m_selectedEntity.has<RotationComponent>())
        {
            if (auto renderer = factory.createRenderer("RotationComponent"))
            {
                renderer->render(m_selectedEntity);
            }
        }

        if (m_selectedEntity.has<ScaleComponent>())
        {
            if (auto renderer = factory.createRenderer("ScaleComponent"))
            {
                renderer->render(m_selectedEntity);
            }
        }

        if (m_selectedEntity.has<CameraComponent>())
        {
            if (auto renderer = factory.createRenderer("CameraComponent"))
            {
                renderer->render(m_selectedEntity);
            }
        }

        if (m_selectedEntity.has<MeshComponent>())
        {
            if (auto renderer = factory.createRenderer("MeshComponent"))
            {
                renderer->render(m_selectedEntity);
            }
        }

        if (m_selectedEntity.has<ScriptComponent>())
        {
            if (auto renderer = factory.createRenderer("ScriptComponent"))
            {
                renderer->render(m_selectedEntity);
            }
        }

        if (m_selectedEntity.has<DirectionalLightComponent>())
        {
            if (auto renderer = factory.createRenderer("DirectionalLightComponent"))
            {
                renderer->render(m_selectedEntity);
            }
        }
        if (m_selectedEntity.has<RigidbodyComponent>())
        {
            if (auto renderer = factory.createRenderer("RigidbodyComponent"))
            {
                renderer->render(m_selectedEntity);
            }
        }

        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("New component   Remove entity").x) / 2);

        if (ImGui::Button("New component"))
        {
            ImGui::OpenPopup("AddComponent");
        }

        renderAddComponentPopup();

        ImGui::SameLine();

        if (ImGui::Button("Remove entity"))
        {
            m_selectedEntity.destruct();
        }
    }
}

void GUIWorldInspector::renderAddComponentPopup()
{
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 3));
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), 0,
                            ImVec2(0.5f, 0.5f));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove;

    if (ImGui::BeginPopupModal("AddComponent", 0, flags))
    {
        if (ImGui::BeginCombo("Select component", "zero comp"))
        {
            auto& registeredComponents = m_ecsModule->getRegisteredComponents();
            for (size_t i = 0; i < registeredComponents.size(); ++i)
            {
                if (ImGui::Selectable(registeredComponents[i].second.c_str()))
                {
                    if (!m_selectedEntity.has(registeredComponents[i].first))
                    {
                        m_selectedEntity.add(registeredComponents[i].first);
                        ImGui::CloseCurrentPopup();
                        break;
                    }
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
