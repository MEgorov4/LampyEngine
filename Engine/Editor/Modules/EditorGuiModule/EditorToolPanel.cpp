#include "EditorToolPanel.h"

#include "Events.h"

#include <EngineMinimal.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/PhysicsModule/PhysicsLocator.h>
#include <Modules/PhysicsModule/PhysicsContext/PhysicsContext.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <imgui.h>
#include <btBulletDynamicsCommon.h>

GUIEditorToolPanel::GUIEditorToolPanel() :
    ImGUIModule::GUIObject(), m_ecsModule(GCM(ECSModule::ECSModule)),
    m_projectModule(GCXM(ProjectModule::ProjectModule))
{
}

void GUIEditorToolPanel::render(float deltaTime)
{
    ZoneScopedN("GUIObject::ToolPanel");
    if (!isVisible())
        return;

    bool windowOpen = true;
    if (ImGui::Begin("Tool panel", &windowOpen, 0))
    {
        // Get simulation status from ECS module (could be replaced with event-based approach)
        bool isSimulating = m_ecsModule->isSimulate();
        
        if (!isSimulating)
        {
            if (ImGui::Button("Start"))
            {
                GCEB().emit(Events::EditorUI::SimulationStart{});
            }
        }

        ImGui::SameLine();

        if (isSimulating)
        {
            if (ImGui::Button("Stop"))
            {
                GCEB().emit(Events::EditorUI::SimulationStop{});
            }
        }

        ImGui::SameLine();

        // Physics Debug Draw toggle
        if (auto* physicsCtx = PhysicsModule::PhysicsLocator::TryGet())
        {
            bool debugDrawEnabled = physicsCtx->isDebugDrawEnabled();
            if (ImGui::Checkbox("Physics Debug", &debugDrawEnabled))
            {
                physicsCtx->setDebugDrawEnabled(debugDrawEnabled);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Toggle physics debug visualization");
            }

            // Debug mode flags (only show when debug is enabled)
            if (debugDrawEnabled)
            {
                ImGui::SameLine();
                if (ImGui::Button("Debug Options..."))
                {
                    ImGui::OpenPopup("PhysicsDebugOptions");
                }

                if (ImGui::BeginPopup("PhysicsDebugOptions"))
                {
                    int currentMode = physicsCtx->getDebugMode();
                    int newMode = currentMode;

                    // Toggle flags
                    bool wireframe = (currentMode & btIDebugDraw::DBG_DrawWireframe) != 0;
                    bool aabb = (currentMode & btIDebugDraw::DBG_DrawAabb) != 0;
                    bool contactPoints = (currentMode & btIDebugDraw::DBG_DrawContactPoints) != 0;
                    bool constraints = (currentMode & btIDebugDraw::DBG_DrawConstraints) != 0;
                    bool normals = (currentMode & btIDebugDraw::DBG_DrawNormals) != 0;

                    if (ImGui::Checkbox("Wireframe", &wireframe))
                    {
                        if (wireframe)
                            newMode |= btIDebugDraw::DBG_DrawWireframe;
                        else
                            newMode &= ~btIDebugDraw::DBG_DrawWireframe;
                    }

                    if (ImGui::Checkbox("AABB", &aabb))
                    {
                        if (aabb)
                            newMode |= btIDebugDraw::DBG_DrawAabb;
                        else
                            newMode &= ~btIDebugDraw::DBG_DrawAabb;
                    }

                    if (ImGui::Checkbox("Contact Points", &contactPoints))
                    {
                        if (contactPoints)
                            newMode |= btIDebugDraw::DBG_DrawContactPoints;
                        else
                            newMode &= ~btIDebugDraw::DBG_DrawContactPoints;
                    }

                    if (ImGui::Checkbox("Constraints", &constraints))
                    {
                        if (constraints)
                            newMode |= btIDebugDraw::DBG_DrawConstraints;
                        else
                            newMode &= ~btIDebugDraw::DBG_DrawConstraints;
                    }

                    if (ImGui::Checkbox("Normals", &normals))
                    {
                        if (normals)
                            newMode |= btIDebugDraw::DBG_DrawNormals;
                        else
                            newMode &= ~btIDebugDraw::DBG_DrawNormals;
                    }

                    if (newMode != currentMode)
                    {
                        physicsCtx->setDebugMode(newMode);
                    }

                    ImGui::EndPopup();
                }
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Save world"))
        {
            std::string editorWorldPath = m_projectModule->getProjectConfig().getEditorStartWorld();
            if (editorWorldPath == "default")
            {
                ImGui::OpenPopup("SelectFolder");
            }
            else
            {
                Events::EditorUI::WorldSaveRequest evt{};
                evt.filePath = editorWorldPath;
                GCEB().emit(evt);
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Save world as"))
        {
            ImGui::OpenPopup("SelectFolder");
        }

        renderSaveWorldPopup();
    }
    
    // Handle window close button
    if (!windowOpen)
    {
        hide();
    }
    
    ImGui::End();
}

void GUIEditorToolPanel::renderSaveWorldPopup()
{
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 3));
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), 0,
                            ImVec2(0.5f, 0.5f));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    if (ImGui::BeginPopupModal("SelectFolder", 0, flags))
    {
        ProjectModule::ProjectConfig& config = m_projectModule->getProjectConfig();

        static std::filesystem::path currentPath(config.getResourcesPath());
        static std::filesystem::path rootPath(config.getResourcesPath());

        ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

        ImGui::BeginChild("FoldersTree", ImVec2(ImGui::GetWindowWidth() * 0.3f, 0));
        ImGui::Text("Folders");
        ImGui::Separator();
        for (const auto& dir : std::filesystem::directory_iterator(currentPath))
        {
            std::string subDirName = dir.path().filename().string();
            if (dir.is_directory())
            {
                if (ImGui::Selectable(subDirName.c_str()))
                {
                    currentPath = currentPath / dir;
                }
            }
        }
        if (currentPath != rootPath)
        {
            if (ImGui::Button("..##"))
            {
                currentPath = currentPath.parent_path();
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("FolderView");
        ImGui::Text(std::string("Selected path: " + std::filesystem::relative(currentPath, rootPath).string()).c_str());

        ImGui::Separator();
        static char buffer[256] = "";
        ImGui::InputText("##", buffer, sizeof(buffer));

        ImGui::SameLine();

        if (ImGui::Button("Save"))
        {
            if (strlen(buffer) > 0)
            {
                std::string worldPath = (currentPath / buffer).string();
                Events::EditorUI::WorldSaveAsRequest evt{};
                evt.filePath = worldPath;
                GCEB().emit(evt);
                
                memset(buffer, 0, sizeof(buffer));
                ImGui::CloseCurrentPopup();
            }
        }

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndChild();
        ImGui::EndPopup();
    }
}
