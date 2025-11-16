#pragma once

#include <EngineMinimal.h>

#include <Foundation/Event/EventBus.h>
#include <Modules/ImGuiModule/GUIObject.h>

#include "../Events.h"
#include <array>
#include <filesystem>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ECSModule
{
class ECSModule;
}

namespace ProjectModule
{
class ProjectModule;
}

namespace ResourceModule
{
class ResourceManager;
class AssetDatabase;
}

class GUIFolderActionPopup;
class IFileAction;

struct DirectoryContent
{
    std::vector<std::filesystem::path> files;
    std::vector<std::filesystem::path> folders;
    uint64_t timestamp = 0;
};

struct DirectoryTreeNode
{
    std::filesystem::path path;
    std::vector<DirectoryTreeNode> children;
    uint64_t timestamp = 0;
    bool isFavorite = false;
    bool expanded = false;
};

class DirectoryCache
{
  public:
    const DirectoryContent &get(const std::filesystem::path &path) const;
    bool contains(const std::filesystem::path &path) const;
    void set(const std::filesystem::path &path, DirectoryContent content);
    void invalidate(const std::filesystem::path &path);
    void invalidateAll();

  private:
    std::unordered_map<std::string, DirectoryContent> m_cache;
};

class DirectoryTreeCache
{
  public:
    DirectoryTreeNode &root();
    const DirectoryTreeNode &root() const;
    void rebuild(const std::filesystem::path &rootPath);
    void invalidate(const std::filesystem::path &path);
    bool empty() const;

  private:
    DirectoryTreeNode m_root;
    bool m_initialized = false;
};

class GUIContentBrowser : public ImGUIModule::GUIObject
{
    ProjectModule::ProjectModule *m_projectModule;

    std::string m_rootPath;
    std::string m_currentPath;
    DirectoryIterator m_dirIter;
    void updateContent();

  public:
    GUIContentBrowser();
    ~GUIContentBrowser() override = default;

    void render(float deltaTime) override;

  private:
    static constexpr size_t kInputBufferSize = 256;

    void renderFilePopup(const std::vector<std::filesystem::path> &targets);

  private:
    enum class CreateEntryType
    {
        Folder = 0,
        File   = 1
    };

    void navigateTo(const std::filesystem::path &path);
    void updateTree();
    void emitDirectoryNavigated();
    void renderFolderNode(DirectoryTreeNode &node, int depth);
    void renderPreviewPane();
    void renderSearchBar();
    void renderFileList();
    void renderTypeFilterBar();
    void renderToolbar();
    void renderTreePane();
    void renderFilesPane();
    void renderSaveAsPopup();
    void openSaveAsPopup(const std::filesystem::path &target, const std::string &guid);
    void clearSelection();
    std::string relativeToRoot(const std::filesystem::path &path) const;
    std::string getAssetGuid(const std::filesystem::path &path) const;
    void loadFavorites();
    void saveFavorites() const;
    bool isFavorite(const std::filesystem::path &path) const;
    void toggleFavorite(const std::filesystem::path &path);
    void renderCreateEntryPopup();
    void requestCreatePopup(CreateEntryType type);
    void rebuildTypeFilters(const DirectoryContent &content);
    bool matchesActiveTypeFilter(const std::filesystem::path &path, bool isDirectory) const;
    std::string normalizeExtension(const std::filesystem::path &path) const;

    DirectoryTreeCache m_treeCache;
    DirectoryCache m_directoryCache;
    std::vector<std::filesystem::path> m_favoriteFolders;
    std::filesystem::path m_editorSettingsPath;
    std::array<char, kInputBufferSize> m_createEntryInput{};
    std::array<char, kInputBufferSize> m_searchInput{};
    std::array<char, kInputBufferSize> m_actionArgumentInput{};
    std::array<char, kInputBufferSize> m_pathInput{};
    std::array<char, kInputBufferSize> m_saveAsInput{};
    std::filesystem::path m_selectedFile;
    std::vector<std::filesystem::path> m_multiSelection;
    std::string m_searchQuery;
    bool m_treeDirty = true;
    int m_anchorIndex = -1;
    EngineCore::Foundation::EventBus::Subscription<Events::EditorUI::FileOperationCompleted> m_fileOperationSub;
    std::unique_ptr<IFileAction> m_pendingArgumentAction;
    std::filesystem::path m_pendingActionTarget;
    std::string m_actionArgumentLabel;
    ResourceModule::ResourceManager *m_resourceManager = nullptr;
    ResourceModule::AssetDatabase *m_assetDatabase = nullptr;
    std::string m_pendingSaveAsGuid;
    bool m_pathEditEnabled = false;
    bool m_focusPathEdit = false;
    CreateEntryType m_pendingCreateType = CreateEntryType::Folder;
    struct TypeFilterOption
    {
        std::string label;
        std::string value;
    };
    std::vector<TypeFilterOption> m_typeFilters;
    int m_typeFilterIndex = 0;
};
