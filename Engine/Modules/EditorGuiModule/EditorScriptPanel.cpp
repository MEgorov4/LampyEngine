#include "EditorScriptPanel.h"
#include "imgui.h"

#include "../FilesystemModule/FilesystemModule.h"
GUIEditorScriptPanel::GUIEditorScriptPanel()
{

	std::string resourcesPath = FS.getCurrentPath() + "\\..\\Resources\\Scripts\\";
	std::vector<std::string> items =
		FS.getDirectoryContents(resourcesPath
			, ContentSearchFilter{ DirContentType::ALL, std::vector<std::string>({".lua"})});
}

void GUIEditorScriptPanel::render()
{
	if (ImGui::Begin("ScriptComponent Panel"))
	{
		if (ImGui::BeginChild("Scripts"))
		{

			ImGui::EndChild();
		}
	}

	ImGui::End();
}
