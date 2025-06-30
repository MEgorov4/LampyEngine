#include "EditorScriptPanel.h"
#include "imgui.h"

#include "../FilesystemModule/FilesystemModule.h"
GUIEditorScriptPanel::GUIEditorScriptPanel(const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule) : m_filesystemModule(filesystemModule)
{
	std::string resourcesPath = m_filesystemModule->getCurrentPath() + "\\..\\Resources\\Scripts\\";
	std::vector<std::string> items =
		m_filesystemModule->getDirectoryContents(resourcesPath
			, FilesystemModule::ContentSearchFilter{ FilesystemModule::DirContentType::ALL, std::vector<std::string>({".lua"})});
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
