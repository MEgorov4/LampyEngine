#include "ContentBrowser.h"
#include <imgui.h>
#include <filesystem>
#include <fstream>
#include "../../ProjectModule/ProjectModule.h"
#include "../../ObjectCoreModule/ECS/ECSModule.h"
#include "FileActionFactory.h"

namespace fs = std::filesystem;

GUIContentBrowser::GUIContentBrowser()
	: GUIObject()
	, m_rootPath(ProjectModule::getInstance().getProjectConfig().getResourcesPath())
	, m_currentPath(m_rootPath)
{
	auto& factory = FileActionFactoryRegistry::getInstance();
	factory.registerFactory(".lworld", [](){return std::make_unique<WorldFileActionFactory>(); });

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
	if (ImGui::Begin("Content Browser"))
	{
		ImGui::BeginChild("FoldersPane", ImVec2(ImGui::GetWindowWidth() * 0.3f, 0), true);
		ImGui::Text("Folders");
		ImGui::Separator();

		renderFolderTree(m_rootPath);

		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("FilesPane", ImVec2(0, 0), true);
		ImGui::Text("Files");
		ImGui::Separator();

		for (size_t i = 0; i < m_files.size(); ++i)
		{
			std::string fullFilePath = normilizePath(m_currentPath + "\\" + m_files[i]);
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

		auto& registry = FileActionFactoryRegistry::getInstance();

		auto factory = registry.getFactory(filePath);
		auto actions = factory->createActions();
		for (auto& action : actions)
		{
			if (ImGui::Button((action->getName() + "##" + filePath).c_str()))
			{
				action->execute(filePath);
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

void GUIContentBrowser::renderFolderTree(const fs::path& directory)
{
	for (const auto& entry : fs::directory_iterator(directory))
	{
		if (entry.is_directory())
		{
			fs::path path = entry.path();
			std::string folderName = path.filename().string();

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
			if (m_currentPath == path.string())
				flags |= ImGuiTreeNodeFlags_Selected;

			bool hasSubDirs = false;
			for (const auto& subEntry : fs::directory_iterator(path))
			{
				if (subEntry.is_directory())
				{
					hasSubDirs = true;
					break;
				}
			}
			if (!hasSubDirs)
				flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			std::string nodeId = folderName + "##" + path.string();

			bool nodeOpen = ImGui::TreeNodeEx(nodeId.c_str(), flags, folderName.c_str());
			if (ImGui::IsItemClicked())
			{
				m_currentPath = path.string();
				updateDirectoryContents();
			}
			if (nodeOpen && hasSubDirs)
			{
				renderFolderTree(path);
				ImGui::TreePop();
			}
		}
	}
}

