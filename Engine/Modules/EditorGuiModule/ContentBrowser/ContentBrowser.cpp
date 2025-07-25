#include "ContentBrowser.h"

#include <imgui.h>

#include "../../ProjectModule/ProjectModule.h"
#include "../../FilesystemModule/DirectoryIterator.h"
#include "../../ObjectCoreModule/ECS/ECSModule.h"

#include "FileActionFactory.h"

GUIContentBrowser::GUIContentBrowser(std::shared_ptr<FilesystemModule::FilesystemModule> filesystemModule,
                                     std::shared_ptr<ProjectModule::ProjectModule> projectModule,
                                     std::shared_ptr<ECSModule::ECSModule> ecsModule)
    : ImGuiModule::GUIObject()
      , m_filesystemModule(filesystemModule)
      , m_projectModule(projectModule)
      , m_ecsModule(ecsModule)
      , m_rootPath(std::string(m_projectModule->getProjectConfig().getResourcesPath()))
      , m_currentPath(m_rootPath)
    ,  m_dirIter(m_filesystemModule->createDirectoryIterator())
{
    auto& factory = FileActionFactoryRegistry::getInstance();
    factory.injectModules(m_projectModule, m_filesystemModule, m_ecsModule);
    factory.registerFactory(".lworld", [this]() { return std::make_unique<WorldFileActionFactory>(m_projectModule, m_filesystemModule, m_ecsModule); });
}

void GUIContentBrowser::updateContent()
{
    std::thread thread([&]()
    {
        m_files.clear();
        m_folders.clear();

        m_folders = m_dirIter.getCurrentDirContents({.contentType = FilesystemModule::DirContentType::FOLDERS});
        m_files = m_dirIter.getCurrentDirContents({.contentType = FilesystemModule::DirContentType::FILES});
    });

    thread.detach();
}

void GUIContentBrowser::render()
{
    if (ImGui::Begin("Content Browser"))
    {
        ImGui::BeginChild("FoldersPane", ImVec2(ImGui::GetWindowWidth() * 0.3f, 0), true);
        ImGui::SetWindowFontScale(1.2f);
        ImGui::Text("Folders");
        ImGui::SetWindowFontScale(1.f);
        ImGui::Separator();

        if (m_dirIter.isCurrentDirChanged())
            updateContent();

        renderFolderTree(m_rootPath);

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("FilesPane", ImVec2(0, 0), true);


        ImGui::SetWindowFontScale(1.2f);
        ImGui::Text("Files in");
        ImGui::SetWindowFontScale(1.f);

        ImGui::Separator();

        renderInFolderFiles();
        ImGui::EndChild();
    }
    ImGui::End();
}

void GUIContentBrowser::renderInFolderFiles()
{
    static std::string selectedFile;
    for (size_t i = 0; i < m_files.size(); ++i)
    {
        if (ImGui::Selectable(m_files[i].c_str()))
        {
            ImGui::OpenPopup(("ItemAction##"));
            selectedFile = m_files[i];
        }

        std::string fullFilePath = m_dirIter.getCurrentDirWithAppend(m_files[i]);

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload("FilePath", fullFilePath.c_str(), fullFilePath.size() + 1);
            ImGui::Text(fullFilePath.c_str());
            ImGui::EndDragDropSource();
        }
    }
    if (!selectedFile.empty())
    {
        renderFilePopup(m_dirIter.getCurrentDirWithAppend(selectedFile));
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
                ImGui::CloseCurrentPopup();
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
                    m_filesystemModule->createDirectory(m_currentPath, strBuffer);
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
                    m_filesystemModule->createFile(m_dirIter.getCurrentDir(), strBuffer);
                    memset(buffer, 0, sizeof(buffer));
                }
            }
            ImGui::EndPopup();
        }
        ImGui::EndPopup();
    }
}

void GUIContentBrowser::renderFolderTree(const std::filesystem::path& directory)
{
    for (auto& dir : m_folders)
    {
        if (ImGui::Selectable(dir.c_str()))
        {
            m_dirIter.stepIntoFolder(dir);
        }
    }

    if (!m_dirIter.isRootPath())
    {
        ImGui::Separator();
        if (ImGui::Button("back"))
        {
            m_dirIter.stepIntoParent();
        }

        ImGui::SameLine();

        if (ImGui::Button("root"))
        {
            m_dirIter.stepIntoRoot();
        }
    }
}
