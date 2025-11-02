#pragma once

#include <EngineMinimal.h>

#include "../../ImGuiModule/GUIObject.h"

namespace ECSModule
{
class ECSModule;
}

namespace ProjectModule
{
class ProjectModule;
}

class GUIFolderActionPopup;

class GUIContentBrowser : public ImGUIModule::GUIObject
{
    ProjectModule::ProjectModule *m_projectModule;

    std::string m_rootPath;
    std::string m_currentPath;
    std::vector<std::string, ProfileAllocator<std::string>> m_files;
    std::vector<std::string, ProfileAllocator<std::string>> m_folders;

    DirectoryIterator m_dirIter;
    void renderFolderTree(const std::filesystem::path &directory);
    void renderInFolderFiles();

    void updateContent();

  public:
    GUIContentBrowser();
    ~GUIContentBrowser() override = default;

    void render(float deltaTime) override;

  private:
    void renderFoldersSectionPopup();
    void renderFilePopup(const std::string &filePath);
    void renderFolderPopup();
};
