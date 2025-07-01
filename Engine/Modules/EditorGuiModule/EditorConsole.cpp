#include "EditorConsole.h"
#include <imgui.h>

#include "../LuaScriptModule/LuaScriptModule.h"

GUIEditorConsole::GUIEditorConsole(const std::shared_ptr<ScriptModule::LuaScriptModule>& scriptModule) : GUIObject(), m_luaScriptModule(scriptModule)
{

}

void GUIEditorConsole::render(float deltaTime)
{
	ImGui::Begin("Console");

	static char buffer[256] = "";

	ImGui::SetNextItemWidth(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 2);
	if (ImGui::InputText("##ConsoleInput", buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		processCommand(buffer);
		memset(buffer, 0, 256);
	}

	ImGui::End();
}

void GUIEditorConsole::processCommand(const std::string& command) const
{
	m_luaScriptModule->processCommand(command);
}


