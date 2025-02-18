#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "../../ImGuiModule/GUIObject.h"
#include "../../FilesystemModule/DirectoryIterator.h"

class GUIFolderActionPopup;

class GUIContentBrowser : public GUIObject
{
    DirectoryIterator dirIter;
    std::string m_rootPath;
    std::string m_currentPath;
    std::vector<std::string> m_files;
    std::vector<std::string> m_folders;

    void renderFolderTree(const std::filesystem::path& directory);
    void renderInFolderFiles();
public:
    GUIContentBrowser();
    ~GUIContentBrowser() override = default;

    void render() override;
private:
    void renderFoldersSectionPopup();
    void renderFilePopup(const std::string& filePath);
    void renderFolderPopup();
};

