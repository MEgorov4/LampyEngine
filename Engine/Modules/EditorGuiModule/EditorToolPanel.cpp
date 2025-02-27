#include "EditorToolPanel.h"
#include <imgui.h>
#include "../ObjectCoreModule/ECS/ECSModule.h"

#include "../ObjectCoreModule/ECS/ECSComponents.h"

#include "../ProjectModule/ProjectModule.h"
#include <filesystem>
#include <format>

GUIEditorToolPanel::GUIEditorToolPanel() : GUIObject(), m_ecsModule(ECSModule::getInstance())
{
}

void GUIEditorToolPanel::render()
{
	if (ImGui::Begin("Tool panel", nullptr, 0))
	{
		if (!m_ecsModule.getTickEnabled())
		{
			if (ImGui::Button("Start"))
			{
				m_ecsModule.startSystems();
			}
		}

		ImGui::SameLine();

		if (m_ecsModule.getTickEnabled())
		{
			if (ImGui::Button("Stop"))
			{
				m_ecsModule.stopSystems();
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Save world"))
		{
			if (!ECSModule::getInstance().isWorldSetted())
			{
				ImGui::OpenPopup("SelectFolder");
			}
			else
			{
				ECSModule::getInstance().saveCurrentWorld();
			}
		}
		renderSaveWorldPopup();
	}
	ImGui::End();
}

void GUIEditorToolPanel::renderSaveWorldPopup()
{
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 3));
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), 0, ImVec2(0.5f, 0.5f));
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove;

	if (ImGui::BeginPopupModal("SelectFolder", 0, flags))
	{
		ProjectConfig& config = ProjectModule::getInstance().getProjectConfig();

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
			ECSModule::getInstance().setCurrentWorldPath((currentPath.string() + "\\" + buffer));
			ECSModule::getInstance().saveCurrentWorld();
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
