#include "MainMenuBar.h"
#include "Events.h"
#include "EditorGUIModule.h"
#include "imgui.h"
#include <Modules/ImGuiModule/ImGuiModule.h>
#include <Core/CoreGlobal.h>
#include <Core/Context.h>

GUIMainMenuBar::GUIMainMenuBar() : GUIObject()
{
}

GUIMainMenuBar::~GUIMainMenuBar()
{
}

void GUIMainMenuBar::render(float deltaTime)
{
    ZoneScopedN("GUIObject::MainMenuBar");
    if (ImGui::BeginMainMenuBar()) 
    {
        if (ImGui::BeginMenu("File")) 
        {
            if (ImGui::MenuItem("Open"))
            {
                GCEB().emit(Events::EditorUI::MenuFileOpenRequest{});
            }
            if (ImGui::MenuItem("Save"))
            {
                GCEB().emit(Events::EditorUI::MenuFileSaveRequest{});
            }
            if (ImGui::MenuItem("Exit"))
            {
                GCEB().emit(Events::EditorUI::MenuFileExitRequest{});
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) 
        {
            if (ImGui::MenuItem("Undo"))
            {
                GCEB().emit(Events::EditorUI::MenuEditUndoRequest{});
            }
            if (ImGui::MenuItem("Redo"))
            {
                GCEB().emit(Events::EditorUI::MenuEditRedoRequest{});
            }
            ImGui::EndMenu();
        }

        renderViewMenu();

        if (ImGui::BeginMenu("Help")) 
        {
            if (ImGui::MenuItem("About"))
            {
                GCEB().emit(Events::EditorUI::MenuHelpAboutRequest{});
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar(); 
    }
}

void GUIMainMenuBar::renderViewMenu()
{
    if (ImGui::BeginMenu("View"))
    {
        // Get EditorGUIModule instance through ContextLocator
        auto* editorGUIModule = GCXM(EditorGUIModule);
        if (!editorGUIModule)
            return;

        const auto& windows = editorGUIModule->getWindows();
        const auto& windowNames = editorGUIModule->getWindowNames();

        // Display menu items for each window
        for (size_t i = 0; i < windows.size() && i < windowNames.size(); ++i)
        {
            if (auto windowPtr = windows[i].lock())
            {
                bool isVisible = windowPtr->isVisible();
                if (ImGui::MenuItem(windowNames[i].c_str(), nullptr, &isVisible))
                {
                    windowPtr->setVisible(isVisible);
                }
            }
            else
            {
                // Window was deleted, show disabled item
                ImGui::MenuItem(windowNames[i].c_str(), nullptr, false, false);
            }
        }

        ImGui::EndMenu();
    }
}
