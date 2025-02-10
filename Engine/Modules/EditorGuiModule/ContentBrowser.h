#pragma once
#include <string>
#include <vector>
#include "../ImGuiModule/GUIObject.h"

class GUIFolderActionPopup;

class GUIContentBrowser : public GUIObject
{
    std::string m_rootPath;
    std::string m_currentPath;
    std::vector<std::string> m_files;
    std::vector<std::string> m_folders;

    void updateDirectoryContents();
    std::string normilizePath(const std::string& path);

public:
    GUIContentBrowser();
    ~GUIContentBrowser() override = default;

    void render() override;
private:
    void renderFoldersSectionPopup();
    void renderFilePopup(const std::string& filePath);
    void renderFolderPopup();
};

