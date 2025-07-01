#pragma once
#include <string>
#include <memory>
#include <vector>
#include <filesystem>
#include "../../ImGuiModule/GUIObject.h"

#include "../../FilesystemModule/DirectoryIterator.h"

namespace ECSModule
{
    class ECSModule;
}

namespace ProjectModule
{
    class ProjectModule;
}

namespace FilesystemModule
{
    class FilesystemModule;
}

class GUIFolderActionPopup;

class GUIContentBrowser : public ImGUIModule::GUIObject
{
    std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
    std::shared_ptr<ProjectModule::ProjectModule> m_projectModule;
    std::shared_ptr<ECSModule::ECSModule>   m_ecsModule;
    FilesystemModule::DirectoryIterator m_dirIter;
    std::string m_rootPath;
    std::string m_currentPath;
    std::vector<std::string> m_files;
    std::vector<std::string> m_folders;

    void renderFolderTree(const std::filesystem::path& directory);
    void renderInFolderFiles();

    void updateContent();
public:
    GUIContentBrowser(std::shared_ptr<FilesystemModule::FilesystemModule> filesystemModule,
                      std::shared_ptr<ProjectModule::ProjectModule> projectModule,
                      std::shared_ptr<ECSModule::ECSModule> ecsModule);
    ~GUIContentBrowser() override = default;

    void render(float deltaTime) override;
private:
    void renderFoldersSectionPopup();
    void renderFilePopup(const std::string& filePath);
    void renderFolderPopup();
};

