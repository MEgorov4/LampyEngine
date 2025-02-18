#include "ContentBrowser.h"
#include <imgui.h>
#include <filesystem>
#include <fstream>
#include "../../ProjectModule/ProjectModule.h"
#include "../../FilesystemModule/DirectoryIterator.h"
#include "../../ObjectCoreModule/ECS/ECSModule.h"
#include "FileActionFactory.h"

namespace fs = std::filesystem;

GUIContentBrowser::GUIContentBrowser()
	: GUIObject()
	, m_rootPath(ProjectModule::getInstance().getProjectConfig().getResourcesPath())
	, m_currentPath(m_rootPath)
{
	auto& factory = FileActionFactoryRegistry::getInstance();
	factory.registerFactory(".lworld", []() {return std::make_unique<WorldFileActionFactory>(); });

}

void GUIContentBrowser::updateContent()
{
	m_files.clear();
	m_folders.clear();

	m_folders = dirIter.getCurrentDirContents({ DirContentType::FOLDERS });
	m_files = dirIter.getCurrentDirContents({DirContentType::FILES});
}

void GUIContentBrowser::render()
{
	if (ImGui::Begin("Content Browser"))
	{
		ImGui::BeginChild("FoldersPane", ImVec2(ImGui::GetWindowWidth() * 0.3f, 0), true);
		ImGui::SetWindowFontScale(1.2);
		ImGui::Text("Folders");
		ImGui::SetWindowFontScale(1);
		ImGui::Separator();

		if (dirIter.isCurrentDirChanged())
			updateContent();

		renderFolderTree(m_rootPath);

		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("FilesPane", ImVec2(0, 0), true);

		ImGui::SetWindowFontScale(1.2);
		ImGui::Text("Files");
		ImGui::SetWindowFontScale(1);

		ImGui::Separator();

		renderInFolderFiles();
		ImGui::EndChild();
	}
	ImGui::End();
}

void GUIContentBrowser::renderInFolderFiles()
{
	for (size_t i = 0; i < m_files.size(); ++i)
	{
		if (ImGui::Selectable(m_files[i].c_str()))
		{
			ImGui::OpenPopup(("ItemAction##"));
		}

		std::string fullFilePath = dirIter.getCurrentDirWithAppend(m_files[i]);

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::SetDragDropPayload("FilePath", fullFilePath.c_str(), fullFilePath.size() + 1);
			ImGui::Text(fullFilePath.c_str());
			ImGui::EndDragDropSource();
		}

		renderFilePopup(dirIter.getCurrentDirWithAppend(m_files[i]));
	}
	renderFolderPopup();
}


void GUIContentBrowser::renderFilePopup(const std::string& filePath)
{
	if (ImGui::BeginPopup(std::string("ItemAction##").c_str()))
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
					FS.createFile(dirIter.getCurrentDir(), strBuffer);
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

	for (auto& dir : m_folders)
	{
		if (ImGui::Selectable(dir.c_str()))
		{
			dirIter.stepIntoFolder(dir);
		}
	}

	if (!dirIter.isRootPath())
	{
		ImGui::Separator();
		if (ImGui::Button("back"))
		{
			dirIter.stepIntoParent();
		}

		ImGui::SameLine();

		if (ImGui::Button("root"))
		{
			dirIter.stepIntoRoot();
		}
	}
}


