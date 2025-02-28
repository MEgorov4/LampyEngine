#include "MainMenuBar.h"
#include "imgui.h"

GUIMainMenuBar::GUIMainMenuBar() : GUIObject()
{
}

void GUIMainMenuBar::render()
{
    if (ImGui::BeginMainMenuBar()) 
    {
        if (ImGui::BeginMenu("File")) 
        {
            if (ImGui::MenuItem("Open"))
            {
                OnOpenClicked("Open button clicked");
            }
            if (ImGui::MenuItem("Save"))
            {
                
            }
            if (ImGui::MenuItem("Exit"))
            {
                
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) 
        {
            if (ImGui::MenuItem("Undo"))
            {
                
            }
            if (ImGui::MenuItem("Redo"))
            {
               
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) 
        {
            if (ImGui::MenuItem("About"))
            {
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar(); 
    }
}
