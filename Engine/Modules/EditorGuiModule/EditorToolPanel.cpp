#include "EditorToolPanel.h"
#include <imgui.h>

#include "../ObjectCoreModule/ECS/ECSModule.h"
#include "../ObjectCoreModule/ECS/Components/ECSComponents.h"

#include "../ProjectModule/ProjectModule.h"
#include "../FilesystemModule/FilesystemModule.h"
#include <filesystem>
#include <format>
#include "../../EngineContext/CoreGlobal.h"

GUIEditorToolPanel::GUIEditorToolPanel() 
	: ImGUIModule::GUIObject()
	, m_ecsModule(GCM(ECSModule::ECSModule))
	, m_projectModule(GCXM(ProjectModule::ProjectModule))
	, m_filesystemModule(GCM(FilesystemModule::FilesystemModule))
{
}

void GUIEditorToolPanel::render(float deltaTime)
{
	if (ImGui::Begin("Tool panel", nullptr, 0))
	{
		if (!m_ecsModule->isSimulate())
		{
			if (ImGui::Button("Start"))
			{
				m_ecsModule->simulate(true);
			}
		}

		ImGui::SameLine();

		if (m_ecsModule->isSimulate())
		{
			if (ImGui::Button("Stop"))
			{
				m_ecsModule->simulate(false);
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
				m_filesystemModule->writeTextFile(editorWorldPath, m_ecsModule->getCurrentWorldData());
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Save world as"))
		{
			ImGui::OpenPopup("SelectFolder");
		}

		renderSaveWorldPopup();
	}
	ImGui::End();
}

void GUIEditorToolPanel::renderSaveWorldPopup()
{
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 3));
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), 0,
		ImVec2(0.5f, 0.5f));
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove;

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

			std::string worldPath = currentPath.string() + "\\" + buffer;
			std::string worldData = m_filesystemModule->readTextFile(worldPath);

			m_ecsModule->openWorld(worldData);

			memset(buffer, 0, sizeof(buffer));
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndChild();
		ImGui::EndPopup();
	}
}
