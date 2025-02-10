#include "ContentBrowser.h"
#include <imgui.h>
#include <filesystem>
#include <fstream>
#include "../ProjectModule/ProjectModule.h"

namespace fs = std::filesystem;

GUIContentBrowser::GUIContentBrowser()
	: GUIObject()
	, m_rootPath(ProjectModule::getInstance().getProjectConfig().getResourcesPath())
	, m_currentPath(m_rootPath)
{
	updateDirectoryContents();
}

void GUIContentBrowser::updateDirectoryContents()
{
	m_files.clear();
	m_folders.clear();

	for (const auto& entry : fs::directory_iterator(m_currentPath))
	{
		if (entry.is_directory())
			m_folders.push_back(entry.path().filename().string());
		else if (entry.is_regular_file())
			m_files.push_back(entry.path().filename().string());
	}
}

std::string GUIContentBrowser::normilizePath(const std::string& path)
{
	return std::filesystem::path(path).make_preferred().string();
}

void GUIContentBrowser::render()
{
	if (ImGui::Begin("Content Browser", nullptr, 0))
	{
		ImGui::BeginChild("FoldersPane", ImVec2(ImGui::GetWindowWidth() * 0.3f, 0), true);
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Folders:").x) / 2);
		ImGui::Text("Folders");
		ImGui::SetCursorPosX(0);
		ImGui::Separator();

		for (size_t i = 0; i < m_folders.size(); ++i)
		{
			std::string label = m_folders[i] + "##folder" + std::to_string(i);
			if (ImGui::Selectable(label.c_str()))
			{
				m_currentPath = (fs::path(m_currentPath) / m_folders[i]).string();
				updateDirectoryContents();
			}
		}

		if (m_currentPath != m_rootPath)
		{
			if (ImGui::Button("..##up"))
			{
				m_currentPath = fs::path(m_currentPath).parent_path().string();
				updateDirectoryContents();
			}
		}
		renderFoldersSectionPopup();
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("FilesPane", ImVec2(0, 0), true);
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Files").x) / 2);
		ImGui::Text("Files");
		ImGui::SetCursorPosX(0);
		ImGui::Separator();

		for (size_t i = 0; i < m_files.size(); ++i)
		{

			const std::string& fullFilePath = normilizePath(m_currentPath + "\\" + m_files[i]).c_str();
			std::string label = m_files[i] + "##file" + std::to_string(i);
			if (ImGui::Selectable(label.c_str()))
			{
				ImGui::OpenPopup(("ItemAction##" + fullFilePath).c_str());
			}
			renderFilePopup(fullFilePath);
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("FilePath", fullFilePath.c_str(), fullFilePath.size() + 1);
				ImGui::Text(fullFilePath.c_str());
				ImGui::EndDragDropSource();
			}
		}
		renderFolderPopup();
		ImGui::EndChild();
	}
	ImGui::End();
}


void GUIContentBrowser::renderFoldersSectionPopup()
{
	if (ImGui::BeginPopupContextWindow(std::string("InFolders").c_str()))
	{
		if (ImGui::Button("Add folder"))
		{
			ImGui::OpenPopup("AddFolderPopup");
		}
		if (ImGui::BeginPopup("AddFolderPopup"))
		{
			static char buffer[128] = "";

			ImGui::InputText("##CreateFolder", buffer, sizeof(buffer));

			ImGui::SameLine();
			std::string strBuffer = buffer;
			if (ImGui::Button("Create##"))
			{
				if (strBuffer.size() > 0)
				{
					fs::create_directory(m_currentPath + "\\" + strBuffer);
					memset(buffer, 0, sizeof(buffer));
				}
			}

			ImGui::EndPopup();
		}
		ImGui::EndPopup();
	}

}

void GUIContentBrowser::renderFilePopup(const std::string& filePath)
{
	if (ImGui::BeginPopup(std::string("ItemAction##" + filePath).c_str()))
	{
		ImGui::SetNextItemWidth(ImGui::GetWindowSize().x);

		if (ImGui::Button(std::string("Delete##" + filePath).c_str()))
		{
			ImGui::OpenPopup("ConfirmDelete");
		}
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y / 2)
			, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize 
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoTitleBar;
		if (ImGui::BeginPopupModal("ConfirmDelete", 0, flags))
		{
			ImGui::Text("Are you shure?");
			if (ImGui::Button("Yes"))
			{
				fs::remove(filePath);
				updateDirectoryContents();

				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("No"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::SetNextItemWidth(ImGui::GetWindowSize().x);

		if (ImGui::Button(std::string("Copy path##" + filePath).c_str()))
		{

		}

		ImGui::SetNextItemWidth(ImGui::GetWindowSize().x);

		if (ImGui::Button(std::string("Dublicate##" + filePath).c_str()))
		{

		}

		if (filePath.size() > 7 && filePath.substr(filePath.size() - 7) == ".lworld")
		{
			ImGui::SetNextItemWidth(ImGui::GetWindowSize().x);

			if (ImGui::Button(std::string("Set as default editor world##" + filePath).c_str()))
			{

			}

			ImGui::SetNextItemWidth(ImGui::GetWindowSize().x);

			if (ImGui::Button(std::string("Set as default game world##" + filePath).c_str()))
			{

			}
		}
		ImGui::EndPopup();
	}
}

void GUIContentBrowser::renderFolderPopup() 
{
	if (ImGui::BeginPopupContextWindow(("FilePopup##" + m_currentPath).c_str()))
	{
		if (ImGui::Button("Add file##"))
		{
			ImGui::OpenPopup("AddFile");
		}
		if (ImGui::BeginPopup("AddFile"))
		{
			static char buffer[128] = "";


			ImGui::InputText("##CreateFile", buffer, sizeof(buffer));

			ImGui::SameLine();
			std::string strBuffer = buffer;
			if (ImGui::Button("Create##"))
			{
				if (strBuffer.size() > 0)
				{
					std::ofstream file(m_currentPath + "\\" + strBuffer);
					file << "";
					file.close();
					updateDirectoryContents();
					ImGui::CloseCurrentPopup();
					memset(buffer, 0, sizeof(buffer));
				}
			}
			ImGui::EndPopup();
		}
		ImGui::EndPopup();
	}
}

