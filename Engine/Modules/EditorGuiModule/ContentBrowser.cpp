#include "ContentBrowser.h"
#include <imgui.h>
#include <filesystem>
#include "../ProjectModule/ProjectModule.h"

namespace fs = std::filesystem;

GUIContentBrowser::GUIContentBrowser()
    : m_rootPath(ProjectModule::getInstance().getProjectConfig().getResourcesPath()),
    m_currentPath(m_rootPath)
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

void GUIContentBrowser::render()
{
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y / 3 * 2));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 3));

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Content Browser", nullptr, windowFlags))
    {
        // Левый блок: папки
        ImGui::BeginChild("FoldersPane", ImVec2(ImGui::GetWindowWidth() * 0.3f, 0), true);
        ImGui::Text("Folders:");

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
        ImGui::EndChild();

        ImGui::SameLine();

        // Правый блок: файлы
        ImGui::BeginChild("FilesPane", ImVec2(0, 0), true);
        ImGui::Text("Files:");

        for (size_t i = 0; i < m_files.size(); ++i)
        {
            std::string label = m_files[i] + "##file" + std::to_string(i);
            if (ImGui::Selectable(label.c_str()))
            {
                // Логика при выборе файла
            }
        }

        ImGui::EndChild();
    }
    ImGui::End();
}


