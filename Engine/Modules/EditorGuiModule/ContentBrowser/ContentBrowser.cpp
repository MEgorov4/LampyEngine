#include "ContentBrowser.h"

#include "FileActionFactory.h"
#include "../Events.h"

#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <imgui.h>
#include <cstring>
#include <algorithm>

GUIContentBrowser::GUIContentBrowser() :
    ImGUIModule::GUIObject(), m_projectModule(GCXM(ProjectModule::ProjectModule)),
    m_rootPath(std::string(m_projectModule->getProjectConfig().getResourcesPath())), m_currentPath(m_rootPath),
    m_dirIter(m_rootPath, m_currentPath)
{
    auto& factory = FileActionFactoryRegistry::getInstance();
    factory.registerFactory(".lworld", []() { return std::make_unique<WorldFileActionFactory>(); });
}

void GUIContentBrowser::updateContent()
{
    m_files.clear();
    m_folders.clear();

    m_folders = Fs::getDirectoryContents(m_dirIter.currentDir(), {.contentType = DirContentType::Folders});
    m_files   = Fs::getDirectoryContents(m_dirIter.currentDir(), {.contentType = DirContentType::Files});
}

void GUIContentBrowser::render(float deltaTime)
{
    if (!isVisible())
        return;

    bool windowOpen = true;
    if (ImGui::Begin("Content Browser", &windowOpen))
    {
        // Update content if directory changed
        if (m_dirIter.isCurrentDirChanged())
        {
            updateContent();
            
            // Emit navigation event
            std::string currentPath = m_dirIter.currentDir();
            std::string relativePath = currentPath.substr(m_rootPath.size());
            std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
            if (relativePath.empty())
                relativePath = "/";
            else if (relativePath[0] != '/')
                relativePath = "/" + relativePath;
                
            Events::EditorUI::DirectoryNavigated evt{};
            evt.currentPath = currentPath;
            evt.relativePath = relativePath;
            GCEB().emit(evt);
        }

        // Folders pane (left side)
        ImGui::BeginChild("FoldersPane", ImVec2(ImGui::GetWindowWidth() * 0.25f, 0), true);
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
        ImGui::SetWindowFontScale(1.15f);
        ImGui::Text("Folders");
        ImGui::SetWindowFontScale(1.f);
        ImGui::PopStyleColor();
        ImGui::Separator();

        renderFolderTree(m_rootPath);
        
        // Context menu for folders pane
        if (ImGui::BeginPopupContextWindow("FoldersPaneContext"))
        {
            if (ImGui::MenuItem("Create Folder"))
            {
                ImGui::OpenPopup("CreateFolderPopup");
            }
            if (ImGui::BeginPopup("CreateFolderPopup"))
            {
                static char folderNameBuffer[256] = "";
                ImGui::Text("Folder Name:");
                ImGui::InputText("##FolderName", folderNameBuffer, sizeof(folderNameBuffer));
                if (ImGui::Button("Create"))
                {
                    if (strlen(folderNameBuffer) > 0)
                    {
                        Events::EditorUI::FolderCreateRequest evt{};
                        evt.parentPath = m_dirIter.currentDir();
                        evt.folderName = folderNameBuffer;
                        GCEB().emit(evt);
                        
                        memset(folderNameBuffer, 0, sizeof(folderNameBuffer));
                        updateContent();
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    memset(folderNameBuffer, 0, sizeof(folderNameBuffer));
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::EndChild();

        ImGui::SameLine();

        // Files pane (right side)
        ImGui::BeginChild("FilesPane", ImVec2(0, 0), true);

        // Header with breadcrumb navigation
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
        ImGui::SetWindowFontScale(1.15f);
        
        // Breadcrumb navigation
        std::string currentPath = m_dirIter.currentDir();
        std::string relativePath;
        
        // Calculate relative path from root
        if (currentPath.size() >= m_rootPath.size() && 
            currentPath.substr(0, m_rootPath.size()) == m_rootPath)
        {
            relativePath = currentPath.substr(m_rootPath.size());
            // Normalize path separators
            std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
            if (relativePath.empty())
                relativePath = "/";
            else if (relativePath[0] != '/')
                relativePath = "/" + relativePath;
        }
        else
        {
            relativePath = currentPath;
        }
        
        ImGui::Text("Files");
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.f);
        
        // Current path display
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::TextWrapped("Path: %s", relativePath.c_str());
        ImGui::PopStyleColor();

        // Navigation buttons
        ImGui::BeginGroup();
        if (ImGui::Button("◀ Back"))
        {
            if (!m_dirIter.isRoot())
            {
                m_dirIter.stepIntoParent();
                updateContent();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("🏠 Root"))
        {
            if (!m_dirIter.isRoot())
            {
                m_dirIter.stepIntoRoot();
                updateContent();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("↻ Refresh"))
        {
            updateContent();
        }
        ImGui::EndGroup();
        
        ImGui::Separator();

        renderInFolderFiles();
        ImGui::EndChild();
    }
    
    // Handle window close button
    if (!windowOpen)
    {
        hide();
    }
    
    ImGui::End();
}

void GUIContentBrowser::renderInFolderFiles()
{
    static std::string selectedFile;
    
    // Display files only (folders are shown in the Folders pane on the left)
    if (m_files.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::Text("No files in this directory");
        ImGui::PopStyleColor();
    }
    else
    {
        for (size_t i = 0; i < m_files.size(); ++i)
        {
            std::string fileDisplay = "📄 " + m_files[i];
            
            bool isSelected = (selectedFile == m_files[i]);
            if (ImGui::Selectable(fileDisplay.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    // Double click to open
                    selectedFile = m_files[i];
                    std::string fullPath = m_dirIter.currentDirAppend(selectedFile);
                    
                    Events::EditorUI::FileOpenRequest evt{};
                    evt.filePath = fullPath;
                    GCEB().emit(evt);
                    
                    ImGui::OpenPopup("ItemAction##");
                }
                else
                {
                    selectedFile = m_files[i];
                }
            }

            std::string fullFilePath = m_dirIter.currentDirAppend(m_files[i]);

            // Drag and drop support
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                ImGui::SetDragDropPayload("FilePath", fullFilePath.c_str(), fullFilePath.size() + 1);
                ImGui::Text("%s", m_files[i].c_str());
                ImGui::EndDragDropSource();
            }

            // Context menu for file
            if (ImGui::BeginPopupContextItem(("FileContext##" + m_files[i]).c_str()))
            {
                if (ImGui::MenuItem("Open"))
                {
                    selectedFile = m_files[i];
                    ImGui::OpenPopup("ItemAction##");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Delete"))
                {
                    // TODO: Implement file deletion
                }
                ImGui::EndPopup();
            }
        }
    }

    // File action popup
    if (!selectedFile.empty())
    {
        renderFilePopup(m_dirIter.currentDirAppend(selectedFile));
    }

    // Context menu for empty space in files pane
    renderFolderPopup();
}

void GUIContentBrowser::renderFilePopup(const std::string& filePath)
{
    if (ImGui::BeginPopup("ItemAction##"))
    {
        ImGui::Text("Actions for: %s", std::filesystem::path(filePath).filename().string().c_str());
        ImGui::Separator();

        auto& registry = FileActionFactoryRegistry::getInstance();
        auto factory = registry.getFactory(filePath);
        
        if (factory)
        {
            auto actions = factory->createActions();
            for (auto& action : actions)
            {
                if (ImGui::MenuItem(action->getName().c_str()))
                {
                    action->execute(filePath);
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No actions available");
        }

        ImGui::EndPopup();
    }
}

void GUIContentBrowser::renderFoldersSectionPopup()
{
    // This method is kept for potential future use but currently handled in render()
}

void GUIContentBrowser::renderFolderPopup()
{
    if (ImGui::BeginPopupContextWindow("FilesPaneContext"))
    {
        if (ImGui::MenuItem("Create File"))
        {
            ImGui::OpenPopup("CreateFilePopup");
        }
        if (ImGui::MenuItem("Create Folder"))
        {
            ImGui::OpenPopup("CreateFolderPopup");
        }
        
        if (ImGui::BeginPopup("CreateFilePopup"))
        {
            static char fileNameBuffer[256] = "";
            ImGui::Text("File Name:");
            ImGui::InputText("##FileName", fileNameBuffer, sizeof(fileNameBuffer));
            if (ImGui::Button("Create"))
            {
                if (strlen(fileNameBuffer) > 0)
                {
                    Events::EditorUI::FileCreateRequest evt{};
                    evt.parentPath = m_dirIter.currentDir();
                    evt.fileName = fileNameBuffer;
                    GCEB().emit(evt);
                    
                    memset(fileNameBuffer, 0, sizeof(fileNameBuffer));
                    updateContent();
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                memset(fileNameBuffer, 0, sizeof(fileNameBuffer));
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        
        if (ImGui::BeginPopup("CreateFolderPopup"))
        {
            static char folderNameBuffer[256] = "";
            ImGui::Text("Folder Name:");
            ImGui::InputText("##FolderName", folderNameBuffer, sizeof(folderNameBuffer));
            if (ImGui::Button("Create"))
            {
                if (strlen(folderNameBuffer) > 0)
                {
                    Events::EditorUI::FolderCreateRequest evt{};
                    evt.parentPath = m_dirIter.currentDir();
                    evt.folderName = folderNameBuffer;
                    GCEB().emit(evt);
                    
                    memset(folderNameBuffer, 0, sizeof(folderNameBuffer));
                    updateContent();
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                memset(folderNameBuffer, 0, sizeof(folderNameBuffer));
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        
        ImGui::EndPopup();
    }
}

void GUIContentBrowser::renderFolderTree(const std::filesystem::path& directory)
{
    // Navigation buttons at the top
    if (!m_dirIter.isRoot())
    {
        if (ImGui::Button("◀ Back"))
        {
            m_dirIter.stepIntoParent();
            updateContent();
        }
        ImGui::SameLine();
        if (ImGui::Button("🏠 Root"))
        {
            m_dirIter.stepIntoRoot();
            updateContent();
        }
        ImGui::Separator();
    }

    // Current directory indicator
    std::string currentDirName = m_dirIter.currentDirName();
    if (currentDirName.empty())
        currentDirName = "Root";
        
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.2f, 1.0f));
    ImGui::Text("📂 %s", currentDirName.c_str());
    ImGui::PopStyleColor();
    
    ImGui::Separator();

    // Display folders
    if (m_folders.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::Text("  No subfolders");
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
        for (auto& dir : m_folders)
        {
            std::string folderDisplay = "  📁 " + dir;
            bool isCurrentFolder = (dir == currentDirName);
            
            if (isCurrentFolder)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
            }
            
            if (ImGui::Selectable(folderDisplay.c_str(), isCurrentFolder))
            {
                m_dirIter.stepIntoFolder(dir);
                updateContent();
            }
            
            if (isCurrentFolder)
            {
                ImGui::PopStyleColor();
            }
        }
        ImGui::PopStyleColor();
    }
}
