#include "EditorConsole.h"

#include <Modules/LuaScriptModule/LuaScriptModule.h>
#include <imgui.h>

GUIEditorConsole::GUIEditorConsole() : GUIObject(), m_luaScriptModule(GCM(ScriptModule::LuaScriptModule))
{
}

void GUIEditorConsole::render(float deltaTime)
{
    if (!isVisible())
        return;

    bool windowOpen = true;
    if (ImGui::Begin("Console", &windowOpen))
    {
        static char buffer[256] = "";

        ImGui::SetNextItemWidth(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 2);
        if (ImGui::InputText("##ConsoleInput", buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            processCommand(buffer);
            memset(buffer, 0, 256);
        }
    }
    
    // Handle window close button
    if (!windowOpen)
    {
        hide();
    }
    
    ImGui::End();
}

void GUIEditorConsole::processCommand(const std::string& command) const
{
    m_luaScriptModule->processCommand(command);
}
