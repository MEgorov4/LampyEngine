#include "ContentBrowser.h"

#include "FileActionFactory.h"
#include "../Events.h"

#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <Foundation/Filesystem/Fs.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <stb_image.h>
#include <array>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <optional>
#include <set>

using namespace EngineCore::Foundation;

namespace
{
constexpr const char* kCreateEntryPopup       = "CB_CreateEntry";
constexpr const char* kItemActionsPopup       = "CB_ItemActions";
constexpr const char* kActionArgumentPopup    = "CB_ActionArgument";
constexpr const char* kSaveAsPopup            = "CB_SaveAssetAs";

struct FileEntry
{
    std::filesystem::path path;
    bool isDirectory = false;
};

bool fuzzyMatch(const std::string& query, const std::string& value)
{
    if (query.empty())
        return true;

    size_t qIndex = 0;
    for (char c : value)
    {
        if (tolower(static_cast<unsigned char>(c)) ==
            tolower(static_cast<unsigned char>(query[qIndex])))
        {
            ++qIndex;
            if (qIndex == query.size())
                return true;
        }
    }
    return false;
}

bool isImageFile(const std::filesystem::path& path)
{
    static const std::vector<std::string> s_imageExt = {".png", ".jpg", ".jpeg", ".bmp", ".tga"};
    const auto ext = path.extension().string();
    return std::any_of(s_imageExt.begin(), s_imageExt.end(),
                       [&](const std::string& value) { return value == ext; });
}

std::string canonicalString(const std::filesystem::path& path)
{
    std::error_code ec;
    auto canonical = std::filesystem::weakly_canonical(path, ec);
    if (ec)
        canonical = path;
    return canonical.generic_string();
}

template <size_t N>
void clearInputBuffer(std::array<char, N>& buffer)
{
    buffer.fill(0);
}

template <size_t N>
void setInputBuffer(const std::string& value, std::array<char, N>& buffer)
{
    clearInputBuffer(buffer);
    if (value.empty())
        return;
    std::strncpy(buffer.data(), value.c_str(), N - 1);
}

} // namespace

const DirectoryContent& DirectoryCache::get(const std::filesystem::path& path) const
{
    static DirectoryContent s_empty;
    const auto key = canonicalString(path);
    const auto it  = m_cache.find(key);
    if (it == m_cache.end())
        return s_empty;
    return it->second;
}

bool DirectoryCache::contains(const std::filesystem::path& path) const
{
    const auto key = canonicalString(path);
    return m_cache.find(key) != m_cache.end();
}

void DirectoryCache::set(const std::filesystem::path& path, DirectoryContent content)
{
    m_cache[canonicalString(path)] = std::move(content);
}

void DirectoryCache::invalidate(const std::filesystem::path& path)
{
    const auto key = canonicalString(path);
    for (auto it = m_cache.begin(); it != m_cache.end();)
    {
        if (it->first.rfind(key, 0) == 0)
            it = m_cache.erase(it);
        else
            ++it;
    }
}

void DirectoryCache::invalidateAll()
{
    m_cache.clear();
}

namespace
{
void buildTreeRecursive(DirectoryTreeNode& node)
{
    node.children.clear();
    auto folders = Fs::getDirectoryContents(node.path.string(), {.contentType = DirContentType::Folders});
    for (const auto& folder : folders)
    {
        DirectoryTreeNode child;
        child.path      = node.path / folder;
        child.timestamp = Fs::folderModTime(child.path.string());
        child.expanded  = false;
        child.isFavorite = false;
        buildTreeRecursive(child);
        node.children.emplace_back(std::move(child));
    }
}
} // namespace

DirectoryTreeNode& DirectoryTreeCache::root()
{
    return m_root;
}

const DirectoryTreeNode& DirectoryTreeCache::root() const
{
    return m_root;
}

void DirectoryTreeCache::rebuild(const std::filesystem::path& rootPath)
{
    m_root.path      = rootPath;
    m_root.timestamp = Fs::folderModTime(rootPath.string());
    m_root.expanded  = true;
    buildTreeRecursive(m_root);
    m_initialized = true;
}

void DirectoryTreeCache::invalidate(const std::filesystem::path& path)
{
    (void)path;
    m_initialized = false;
}

bool DirectoryTreeCache::empty() const
{
    return !m_initialized;
}

GUIContentBrowser::GUIContentBrowser() :
    ImGUIModule::GUIObject(),
    m_projectModule(GCXM(ProjectModule::ProjectModule)),
    m_rootPath(std::string(m_projectModule->getProjectConfig().getResourcesPath())),
    m_currentPath(m_rootPath), m_dirIter(m_rootPath, m_currentPath)
{
    auto& factory = FileActionFactoryRegistry::getInstance();
    factory.registerFactory(".lworld", []() { return std::make_unique<WorldFileActionFactory>(); });

    clearInputBuffer(m_createEntryInput);
    clearInputBuffer(m_searchInput);
    clearInputBuffer(m_actionArgumentInput);
    clearInputBuffer(m_pathInput);
    m_selectedFile.clear();
    m_multiSelection.clear();
    m_searchQuery.clear();
    m_anchorIndex = -1;
    m_pendingArgumentAction.reset();
    m_pendingActionTarget.clear();
    m_actionArgumentLabel.clear();
    m_pathEditEnabled = false;
    m_focusPathEdit   = false;

    m_editorSettingsPath = std::filesystem::path(m_projectModule->getProjectConfig().getConfigPath()) /
                           "EditorSettings.json";

    m_resourceManager = GCM(ResourceModule::ResourceManager);
    if (m_resourceManager)
    {
        m_assetDatabase = m_resourceManager->getDatabase();
        if (!m_assetDatabase)
        {
            LT_LOG(LogVerbosity::Warning, "ContentBrowser", "Asset database is not available");
        }
    }
    else
    {
        LT_LOG(LogVerbosity::Warning, "ContentBrowser", "ResourceManager not available");
    }

    loadFavorites();
    m_fileOperationSub =
        GCEB().subscribe<Events::EditorUI::FileOperationCompleted>([this](const auto& evt) {
            std::filesystem::path changedPath = evt.filePath;
            if (changedPath.has_parent_path())
                changedPath = changedPath.parent_path();
            if (changedPath.empty())
                changedPath = m_currentPath;
            m_directoryCache.invalidate(changedPath);
            m_treeDirty = true;
        });
    navigateTo(m_rootPath);
}

void GUIContentBrowser::updateContent()
{
    const std::filesystem::path currentPath = m_dirIter.currentDir();
    const uint64_t timestamp                = Fs::folderModTime(currentPath.string());

    const bool hasCached = m_directoryCache.contains(currentPath);
    const auto& cached   = m_directoryCache.get(currentPath);
    if (hasCached && cached.timestamp == timestamp && timestamp != 0)
        return;

    DirectoryContent refreshed;
    refreshed.timestamp = timestamp;

    auto folders = Fs::getDirectoryContents(currentPath.string(), {.contentType = DirContentType::Folders});
    for (const auto& folder : folders)
    {
        refreshed.folders.emplace_back(currentPath / folder);
    }

    auto files = Fs::getDirectoryContents(currentPath.string(), {.contentType = DirContentType::Files});
    for (const auto& file : files)
    {
        refreshed.files.emplace_back(currentPath / file);
    }

    m_directoryCache.set(currentPath, std::move(refreshed));
}

void GUIContentBrowser::render(float deltaTime)
{
    ZoneScopedN("GUIObject::ContentBrowser");
    if (!isVisible())
        return;

    updateTree();
    updateContent();

    bool windowOpen = true;
    if (ImGui::Begin("Content Browser", &windowOpen))
    {
        renderToolbar();

        constexpr float previewHeight = 170.f;
        ImVec2 contentAvail           = ImGui::GetContentRegionAvail();
        float spacing                 = ImGui::GetStyle().ItemSpacing.y;
        float upperHeight             = std::max(0.f, contentAvail.y - previewHeight - spacing);

        ImGui::BeginChild("ContentBrowserUpperRegion", ImVec2(0.f, upperHeight), false);
        {
            const float leftWidth = std::max(220.f, ImGui::GetContentRegionAvail().x * 0.28f);
            ImGui::BeginChild("ContentBrowserTreePane", ImVec2(leftWidth, 0.f), true);
            renderTreePane();
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("ContentBrowserFilesPane", ImVec2(0.f, 0.f), true);
            renderFilesPane();
            ImGui::EndChild();
        }
        ImGui::EndChild();

        ImGui::BeginChild("ContentBrowserPreviewPane", ImVec2(0.f, previewHeight), true);
        renderPreviewPane();
        ImGui::EndChild();
    }

    if (!windowOpen)
    {
        hide();
    }

    ImGui::End();
}

void GUIContentBrowser::navigateTo(const std::filesystem::path& path)
{
    if (path.empty())
        return;

    if (m_dirIter.navigateTo(path.string()) != FsResult::Success)
        return;

    m_currentPath = m_dirIter.currentDir();
    clearSelection();
            updateContent();
    emitDirectoryNavigated();
}

void GUIContentBrowser::updateTree()
{
    if (!m_treeDirty && !m_treeCache.empty())
        return;

    m_treeCache.rebuild(m_dirIter.rootDir());
    m_treeDirty = false;
}

void GUIContentBrowser::emitDirectoryNavigated()
{
    std::string currentPath  = m_dirIter.currentDir();
            std::string relativePath = currentPath.substr(m_rootPath.size());
            std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
            if (relativePath.empty())
                relativePath = "/";
            else if (relativePath[0] != '/')
                relativePath = "/" + relativePath;
                
            Events::EditorUI::DirectoryNavigated evt{};
    evt.currentPath  = currentPath;
            evt.relativePath = relativePath;
            GCEB().emit(evt);
        }

std::string GUIContentBrowser::relativeToRoot(const std::filesystem::path& path) const
{
    std::error_code ec;
    auto rel = std::filesystem::relative(path, m_rootPath, ec);
    if (ec)
        return {};
    auto normalized = rel.generic_string();
    if (normalized == ".")
        normalized.clear();
    return normalized;
}

std::string GUIContentBrowser::getAssetGuid(const std::filesystem::path& path) const
{
    if (!m_assetDatabase)
        return {};

    const auto relative = relativeToRoot(path);
    if (relative.empty())
        return {};

    auto info = m_assetDatabase->findBySource(relative);
    if (!info)
        return {};
    return info->guid.str();
}

void GUIContentBrowser::renderFolderNode(DirectoryTreeNode& node, int depth)
{
    (void)depth;
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (node.children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    const bool isCurrent = canonicalString(node.path) == canonicalString(m_currentPath);
    if (isCurrent)
        flags |= ImGuiTreeNodeFlags_Selected;

    std::string label = node.path.filename().string();
    if (label.empty())
        label = "Root";

    const std::string nodeId = canonicalString(node.path);
    ImGui::PushID(nodeId.c_str());
    bool open = ImGui::TreeNodeEx(label.c_str(), flags);
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        navigateTo(node.path);
    }

    ImGui::SameLine();
    const char* starLabel = isFavorite(node.path) ? "[*]" : "[ ]";
    if (ImGui::SmallButton(starLabel))
    {
        toggleFavorite(node.path);
        saveFavorites();
    }

    if (open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        for (auto& child : node.children)
        {
            renderFolderNode(child, depth + 1);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void GUIContentBrowser::renderSearchBar()
{
    if (ImGui::InputTextWithHint("##ContentBrowserSearch", "Quick search...", m_searchInput.data(),
                                 m_searchInput.size(), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        m_searchQuery = m_searchInput.data();
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("✕"))
    {
        m_searchQuery.clear();
        clearInputBuffer(m_searchInput);
    }
}

void GUIContentBrowser::renderToolbar()
{
    const auto& currentContent = m_directoryCache.get(m_dirIter.currentDir());
    rebuildTypeFilters(currentContent);

    const bool atRoot = canonicalString(std::filesystem::path(m_currentPath)) ==
                        canonicalString(std::filesystem::path(m_rootPath));

    const float toolbarHeight = ImGui::GetFrameHeightWithSpacing() * 2.25f;
    ImGui::BeginChild("ContentBrowserToolbar", ImVec2(0.f, toolbarHeight), true);

    ImGui::BeginDisabled(atRoot);
    if (ImGui::SmallButton("◀"))
    {
        navigateTo(std::filesystem::path(m_currentPath).parent_path());
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Go to parent folder");
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::SmallButton("🏠"))
    {
        navigateTo(m_rootPath);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Go to root");

    ImGui::SameLine();
    if (ImGui::SmallButton("↻"))
    {
        m_directoryCache.invalidate(m_currentPath);
        updateContent();
        m_treeDirty = true;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Refresh current directory");

    ImGui::SameLine();
    if (ImGui::SmallButton("+ File"))
    {
        requestCreatePopup(CreateEntryType::File);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Create new file");

    ImGui::SameLine();
    if (ImGui::SmallButton("+ Folder"))
    {
        requestCreatePopup(CreateEntryType::Folder);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Create new folder");

    std::error_code relEc;
    std::filesystem::path relativePath =
        std::filesystem::relative(std::filesystem::path(m_currentPath), std::filesystem::path(m_rootPath), relEc);
    std::vector<std::pair<std::string, std::filesystem::path>> breadcrumbs;
    breadcrumbs.push_back({"Root", m_rootPath});
    if (!relEc)
    {
        std::filesystem::path aggregate;
        for (const auto& part : relativePath)
        {
            if (part == ".")
                continue;
            aggregate /= part;
            breadcrumbs.push_back({part.string(), std::filesystem::path(m_rootPath) / aggregate});
        }
    }

    ImGui::Spacing();
    if (m_pathEditEnabled)
    {
        if (m_focusPathEdit)
            ImGui::SetKeyboardFocusHere();

        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
        if (ImGui::InputText("##ContentBrowserPathEdit", m_pathInput.data(), m_pathInput.size(), flags))
        {
            std::filesystem::path submitted = std::filesystem::path(m_pathInput.data());
            if (submitted.empty())
                submitted = std::filesystem::path(m_rootPath);
            if (!submitted.is_absolute())
                submitted = std::filesystem::path(m_rootPath) / submitted;

            if (Fs::exists(submitted.string()) && Fs::isDirectory(submitted.string()))
            {
                navigateTo(submitted);
            }
            else
            {
                const std::string message = "Invalid path: " + submitted.string();
                LT_LOG(LogVerbosity::Warning, "ContentBrowser", message.c_str());
            }
            m_pathEditEnabled = false;
        }

        if (!ImGui::IsItemActive() && !m_focusPathEdit)
            m_pathEditEnabled = false;

        if (ImGui::IsItemDeactivated())
            m_focusPathEdit = false;
    }
    else
    {
        ImGui::PushID("Breadcrumbs");
        ImGui::BeginGroup();
        for (size_t i = 0; i < breadcrumbs.size(); ++i)
        {
            if (i > 0)
            {
                ImGui::SameLine();
                ImGui::TextUnformatted(">");
                ImGui::SameLine();
            }
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::SmallButton(breadcrumbs[i].first.c_str()))
            {
                navigateTo(breadcrumbs[i].second);
            }
            ImGui::PopID();
        }
        ImGui::EndGroup();

        if (ImGui::BeginPopupContextItem("CB_BreadcrumbsContext"))
        {
            if (ImGui::MenuItem("Copy Path"))
            {
                Fs::copyAbsolutePathToClipboard(m_currentPath);
            }
            if (ImGui::MenuItem("Edit Path"))
            {
                setInputBuffer(m_currentPath, m_pathInput);
                m_pathEditEnabled = true;
                m_focusPathEdit   = true;
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGui::SameLine();
    renderSearchBar();

    ImGui::SameLine();
    renderTypeFilterBar();

    renderCreateEntryPopup();

    ImGui::EndChild();
    ImGui::Spacing();
}

void GUIContentBrowser::renderTreePane()
{
    ImGui::TextUnformatted("Favorites");
    ImGui::Separator();

    if (m_favoriteFolders.empty())
    {
        ImGui::TextDisabled("No favorites yet");
        }
        else
    {
        for (size_t idx = 0; idx < m_favoriteFolders.size(); ++idx)
        {
            const auto& favorite = m_favoriteFolders[idx];
            std::string displayName = favorite.filename().string();
            if (displayName.empty())
                displayName = favorite.generic_string();

            ImGui::PushID(static_cast<int>(idx));
            if (ImGui::Selectable(displayName.c_str(), false))
            {
                navigateTo(favorite);
            }
            ImGui::SameLine();
            const char* starLabel = isFavorite(favorite) ? "[*]" : "[ ]";
            if (ImGui::SmallButton(starLabel))
            {
                toggleFavorite(favorite);
                saveFavorites();
                m_treeDirty = true;
            }
            ImGui::PopID();
        }
    }

    ImGui::Dummy(ImVec2(0.f, 6.f));
    ImGui::TextUnformatted("Folders");
    ImGui::Separator();

    if (!m_treeCache.empty())
    {
        renderFolderNode(m_treeCache.root(), 0);
    }
    else
    {
        ImGui::TextDisabled("Building tree...");
    }

    if (ImGui::Button("New Folder"))
    {
        requestCreatePopup(CreateEntryType::Folder);
    }
}

void GUIContentBrowser::renderFilesPane()
{
    renderFileList();
}

void GUIContentBrowser::requestCreatePopup(CreateEntryType type)
{
    m_pendingCreateType = type;
    clearInputBuffer(m_createEntryInput);
    ImGui::OpenPopup(kCreateEntryPopup);
}

void GUIContentBrowser::renderCreateEntryPopup()
{
    if (!ImGui::BeginPopup(kCreateEntryPopup))
        return;

    ImGui::InputText("Name", m_createEntryInput.data(), m_createEntryInput.size());

    const char* typeLabels[] = {"Folder", "File"};
    int typeIndex            = (m_pendingCreateType == CreateEntryType::Folder) ? 0 : 1;
    if (ImGui::Combo("Type", &typeIndex, typeLabels, IM_ARRAYSIZE(typeLabels)))
        m_pendingCreateType = (typeIndex == 0) ? CreateEntryType::Folder : CreateEntryType::File;

    auto closePopup = [&]() {
        clearInputBuffer(m_createEntryInput);
        ImGui::CloseCurrentPopup();
    };

    if (ImGui::Button("Create"))
    {
        const std::string name = m_createEntryInput.data();
        bool success           = false;
        if (!name.empty())
        {
            auto parent = std::filesystem::path(m_currentPath);
            auto emitCompletion = [&](const std::filesystem::path& targetPath, const char* operation, bool opSuccess) {
                Events::EditorUI::FileOperationCompleted evt{};
                evt.operation = operation;
                evt.filePath  = targetPath.string();
                evt.success   = opSuccess;
                GCEB().emit(evt);
            };
            if (m_pendingCreateType == CreateEntryType::Folder)
            {
                auto newFolder = parent / name;
                if (Fs::exists(newFolder.string()))
                {
                    const std::string message = "Folder already exists: " + newFolder.string();
                    LT_LOG(LogVerbosity::Warning, "ContentBrowser", message.c_str());
                    emitCompletion(newFolder, "create_folder", false);
                }
                else
                {
                    const auto result = Fs::createDirectory(newFolder.string());
                    if (result == FsResult::Success)
                    {
                        const std::string message = "Created folder: " + newFolder.string();
                        LT_LOG(LogVerbosity::Info, "ContentBrowser", message.c_str());
                        m_directoryCache.invalidate(parent);
                        m_treeDirty = true;
                        updateContent();
                        emitCompletion(newFolder, "create_folder", true);
                        success = true;
                    }
                    else
                    {
                        const std::string message =
                            "Failed to create folder " + newFolder.string() + " (code=" +
                            std::to_string(static_cast<int>(result)) + ")";
                        LT_LOG(LogVerbosity::Error, "ContentBrowser", message.c_str());
                        emitCompletion(newFolder, "create_folder", false);
                    }
                }
            }
            else
            {
                auto newFile = parent / name;
                if (Fs::exists(newFile.string()))
                {
                    const std::string message = "File already exists: " + newFile.string();
                    LT_LOG(LogVerbosity::Warning, "ContentBrowser", message.c_str());
                    emitCompletion(newFile, "create_file", false);
                }
                else
                {
                    const auto result = Fs::createEmptyFile(parent.string(), name);
                    if (result == FsResult::Success)
                    {
                        const std::string message = "Created file: " + newFile.string();
                        LT_LOG(LogVerbosity::Info, "ContentBrowser", message.c_str());
                        m_directoryCache.invalidate(parent);
                        updateContent();
                        emitCompletion(newFile, "create_file", true);
                        success = true;
                    }
                    else
                    {
                        const std::string message =
                            "Failed to create file " + newFile.string() + " (code=" +
                            std::to_string(static_cast<int>(result)) + ")";
                        LT_LOG(LogVerbosity::Error, "ContentBrowser", message.c_str());
                        emitCompletion(newFile, "create_file", false);
                    }
                }
            }
        }

        if (success)
            closePopup();
    }

        ImGui::SameLine();
    if (ImGui::Button("Cancel"))
        closePopup();

    ImGui::EndPopup();
}

std::string GUIContentBrowser::normalizeExtension(const std::filesystem::path& path) const
{
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}

void GUIContentBrowser::rebuildTypeFilters(const DirectoryContent& content)
{
    std::set<std::string> uniqueExtensions;
    for (const auto& file : content.files)
        uniqueExtensions.insert(normalizeExtension(file));

    m_typeFilters.clear();
    m_typeFilters.push_back({"All (*.*)", ""});
    for (const auto& ext : uniqueExtensions)
    {
        if (ext.empty())
            m_typeFilters.push_back({"<no extension>", "<none>"}); 
        else
            m_typeFilters.push_back({ext, ext});
    }

    if (m_typeFilterIndex >= static_cast<int>(m_typeFilters.size()))
        m_typeFilterIndex = 0;
}

bool GUIContentBrowser::matchesActiveTypeFilter(const std::filesystem::path& path, bool isDirectory) const
{
    if (isDirectory)
        return true;
    if (m_typeFilters.empty())
        return true;
    if (m_typeFilterIndex <= 0 || m_typeFilterIndex >= static_cast<int>(m_typeFilters.size()))
        return true;

    std::string currentExt = normalizeExtension(path);
    const std::string& filterValue = m_typeFilters[m_typeFilterIndex].value;
    if (filterValue.empty())
        return true;
    if (filterValue == "<none>")
        return currentExt.empty();
    return currentExt == filterValue;
}

void GUIContentBrowser::renderTypeFilterBar()
{
    if (m_typeFilters.size() <= 1)
        return;

    const std::string displayLabel = "Filter: " + m_typeFilters[m_typeFilterIndex].label;
    ImGui::SetNextItemWidth(160.f);
    if (ImGui::BeginCombo("##CBTypeFilters", displayLabel.c_str()))
    {
        for (int i = 0; i < static_cast<int>(m_typeFilters.size()); ++i)
        {
            bool selected = (i == m_typeFilterIndex);
            if (ImGui::Selectable(m_typeFilters[i].label.c_str(), selected))
                m_typeFilterIndex = i;
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Filter files by extension");
}

void GUIContentBrowser::renderFileList()
{
    const auto& content = m_directoryCache.get(m_dirIter.currentDir());

    std::vector<FileEntry> entries;
    entries.reserve(content.folders.size() + content.files.size());
    for (const auto& folder : content.folders)
        entries.push_back({folder, true});
    for (const auto& file : content.files)
        entries.push_back({file, false});

    std::sort(entries.begin(), entries.end(), [](const FileEntry& lhs, const FileEntry& rhs) {
        if (lhs.isDirectory != rhs.isDirectory)
            return lhs.isDirectory && !rhs.isDirectory;
        return lhs.path.filename().string() < rhs.path.filename().string();
    });

    std::vector<FileEntry> filtered;
    filtered.reserve(entries.size());
    for (const auto& entry : entries)
    {
        const std::string name = entry.path.filename().string();
        if (matchesActiveTypeFilter(entry.path, entry.isDirectory) && fuzzyMatch(m_searchQuery, name))
            filtered.push_back(entry);
    }

    ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;
    if (ImGui::BeginTable("ContentBrowserTable", 3, tableFlags))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.f);
        ImGui::TableSetupColumn("UUID", ImGuiTableColumnFlags_WidthFixed, 220.f);
        ImGui::TableHeadersRow();

        const ImGuiIO& io        = ImGui::GetIO();
        const bool windowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        for (size_t index = 0; index < filtered.size(); ++index)
        {
            auto& entry = filtered[index];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            auto isPathSelected = [&](const std::filesystem::path& candidate) {
                const auto canonicalCandidate = canonicalString(candidate);
                return std::any_of(m_multiSelection.begin(), m_multiSelection.end(),
                                   [&](const std::filesystem::path& other) {
                                       return canonicalString(other) == canonicalCandidate;
                                   });
            };

            const bool selected = isPathSelected(entry.path);
            ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick |
                                                   ImGuiSelectableFlags_SpanAllColumns;
            std::string label = entry.path.filename().string();
            if (label.empty())
                label = entry.isDirectory ? "[Folder]" : "[File]";

            if (ImGui::Selectable(label.c_str(), selected, selectableFlags))
            {
                const bool ctrl  = windowFocused && io.KeyCtrl;
                const bool shift = windowFocused && io.KeyShift;

                auto addSelection = [&](const std::filesystem::path& path) {
                    if (!isPathSelected(path))
                        m_multiSelection.push_back(path);
                };
                auto removeSelection = [&](const std::filesystem::path& path) {
                    const auto canonicalTarget = canonicalString(path);
                    m_multiSelection.erase(std::remove_if(m_multiSelection.begin(), m_multiSelection.end(),
                                                          [&](const std::filesystem::path& other) {
                                                              return canonicalString(other) == canonicalTarget;
                                                          }),
                                           m_multiSelection.end());
                };

                if (m_anchorIndex >= static_cast<int>(filtered.size()))
                    m_anchorIndex = static_cast<int>(filtered.size()) - 1;

                if (shift && m_anchorIndex >= 0 && m_anchorIndex < static_cast<int>(filtered.size()))
                {
                    m_multiSelection.clear();
                    const int start = std::min(m_anchorIndex, static_cast<int>(index));
                    const int end   = std::max(m_anchorIndex, static_cast<int>(index));
                    for (int i = start; i <= end; ++i)
                        m_multiSelection.push_back(filtered[i].path);
                }
                else if (ctrl)
                {
                    if (selected)
                        removeSelection(entry.path);
                    else
                        addSelection(entry.path);
                    m_anchorIndex = static_cast<int>(index);
                }
                else
                {
                    clearSelection();
                    addSelection(entry.path);
                    m_anchorIndex = static_cast<int>(index);
                }

                m_selectedFile = entry.path;

                if (ImGui::IsMouseDoubleClicked(0))
                {
                    if (entry.isDirectory)
                    {
                        navigateTo(entry.path);
                    }
                    else
                    {
                        Events::EditorUI::FileOpenRequest evt{};
                        evt.filePath = entry.path.string();
                        GCEB().emit(evt);
                    }
                }
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (!selected)
                {
                    clearSelection();
                    m_multiSelection.push_back(entry.path);
                    m_selectedFile = entry.path;
                    m_anchorIndex  = static_cast<int>(index);
                }
                ImGui::OpenPopup(kItemActionsPopup);
                ImGui::EndPopup();
            }

            if (ImGui::BeginDragDropSource())
            {
                std::vector<std::filesystem::path> dragTargets;
                if (selected && m_multiSelection.size() > 1)
                    dragTargets = m_multiSelection;
                else
                    dragTargets.push_back(entry.path);

                std::string payload;
                for (size_t i = 0; i < dragTargets.size(); ++i)
                {
                    payload += dragTargets[i].string();
                    if (i + 1 < dragTargets.size())
                        payload.push_back('\n');
                }

                ImGui::SetDragDropPayload("FilePathList", payload.c_str(), payload.size() + 1);
                ImGui::Text("%zu item(s)", dragTargets.size());
                ImGui::EndDragDropSource();
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(entry.isDirectory ? "Folder" : "File");

            ImGui::TableSetColumnIndex(2);
            if (!entry.isDirectory)
            {
                const std::string guid = getAssetGuid(entry.path);
                if (!guid.empty())
                {
                    ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f), "%s", guid.c_str());
                }
                else
                {
                    ImGui::TextDisabled("not indexed");
                }
            }
            else
            {
                ImGui::TextDisabled("-");
            }
        }

        ImGui::EndTable();

        if (windowFocused && io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A))
        {
            clearSelection();
            for (const auto& entry : filtered)
                m_multiSelection.push_back(entry.path);
            if (!m_multiSelection.empty())
            {
                m_selectedFile = m_multiSelection.back();
                m_anchorIndex  = static_cast<int>(filtered.size()) - 1;
            }
        }
    }

    std::vector<std::filesystem::path> targets = m_multiSelection;
    if (targets.empty() && !m_selectedFile.empty())
        targets.push_back(m_selectedFile);

    renderFilePopup(targets);
}

void GUIContentBrowser::renderPreviewPane()
{
    std::filesystem::path target;
    if (!m_multiSelection.empty())
        target = m_multiSelection.front();
    else
        target = m_selectedFile;

    if (target.empty())
    {
        renderSaveAsPopup();
        return;
    }

    ImGui::TextUnformatted("Preview");
    ImGui::Separator();

    if (!Fs::exists(target.string()))
    {
        ImGui::TextDisabled("Missing: %s", target.string().c_str());
        renderSaveAsPopup();
        return;
    }

    if (std::filesystem::is_directory(target))
    {
        ImGui::Text("Folder: %s", target.filename().string().c_str());
        renderSaveAsPopup();
        return;
    }

    if (isImageFile(target))
    {
        int w = 0;
        int h = 0;
        int comp = 0;
        if (stbi_info(target.string().c_str(), &w, &h, &comp))
        {
            ImGui::Text("Image %dx%d (%d ch)", w, h, comp);
            ImVec2 size(120.f, 90.f);
            ImVec2 cursor = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(
                cursor, ImVec2(cursor.x + size.x, cursor.y + size.y), IM_COL32(80, 80, 80, 255));
            ImGui::Dummy(size);
        }
        else
        {
            ImGui::TextDisabled("Unable to read image metadata");
        }
    }
    else if (target.extension() == ".lworld")
    {
        ImGui::Text("World asset");
        ImGui::Text("Path: %s", target.filename().string().c_str());
    }
    else
    {
        ImGui::Text("File: %s", target.filename().string().c_str());
        ImGui::Text("Size: %llu bytes",
                    static_cast<unsigned long long>(Fs::fileSize(target.string())));
    }

    const std::string guid = getAssetGuid(target);
    if (!guid.empty() && m_resourceManager)
    {
        ImGui::Separator();
        ImGui::Text("Asset GUID: %s", guid.c_str());
        if (ImGui::Button("Save Asset"))
        {
            bool saved = m_resourceManager->save(ResourceModule::AssetID(guid));
            if (saved)
            {
                LT_LOG(LogVerbosity::Info, "ContentBrowser",
                       ("Saved asset: " + target.filename().string()).c_str());
            }
            else
            {
                LT_LOG(LogVerbosity::Error, "ContentBrowser", "Failed to save asset");
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save Asset As..."))
        {
            openSaveAsPopup(target, guid);
        }
    }

    renderSaveAsPopup();
}

void GUIContentBrowser::clearSelection()
{
    m_multiSelection.clear();
    m_selectedFile.clear();
    m_anchorIndex = -1;
}

void GUIContentBrowser::renderFilePopup(const std::vector<std::filesystem::path>& targets)
{
    if (targets.empty())
        return;

    bool actionsOpen = ImGui::BeginPopup(kItemActionsPopup);
    if (actionsOpen)
    {
        auto invokeAction = [](const std::filesystem::path& targetPath, const std::string& actionName) {
            auto& registry = FileActionFactoryRegistry::getInstance();
            auto factory   = registry.getFactory(targetPath.string());
            if (!factory)
                return;
            auto actions = factory->createActions();
            for (auto& action : actions)
            {
                if (action->getName() == actionName)
                {
                    action->execute(targetPath.string());
                    break;
                }
            }
        };

        if (targets.size() == 1)
        {
            const auto& target = targets.front();
            ImGui::Text("Actions for: %s", target.filename().string().c_str());
            ImGui::Separator();

            auto& registry = FileActionFactoryRegistry::getInstance();
            auto factory   = registry.getFactory(target.string());
            if (factory)
            {
                auto actions = factory->createActions();
                for (auto& action : actions)
                {
                    if (!action)
                        continue;

                    if (action->requiresArgument())
                    {
                        if (ImGui::MenuItem(action->getName().c_str()))
                        {
                            m_pendingArgumentAction = std::move(action);
                            m_pendingActionTarget   = target;
                            setInputBuffer(target.filename().string(), m_actionArgumentInput);
                            m_actionArgumentLabel   = m_pendingArgumentAction->argumentLabel();
                            ImGui::CloseCurrentPopup();
                            ImGui::OpenPopup(kActionArgumentPopup);
                        }
                    }
                    else if (ImGui::MenuItem(action->getName().c_str()))
                    {
                        action->execute(target.string());
                        ImGui::CloseCurrentPopup();
                        break;
                    }
                }
            }
            else
            {
                ImGui::TextDisabled("No actions available");
            }
        }
        else
        {
            ImGui::Text("Multi-selection (%zu items)", targets.size());
            ImGui::Separator();
            if (ImGui::MenuItem("Delete selected"))
            {
                for (const auto& target : targets)
                    invokeAction(target, "Delete");
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Duplicate selected"))
            {
                for (const auto& target : targets)
                    invokeAction(target, "Duplicate");
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Copy absolute paths"))
            {
                for (const auto& target : targets)
                    invokeAction(target, "Copy absolute path");
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup(kActionArgumentPopup))
    {
        ImGui::TextUnformatted(m_actionArgumentLabel.empty() ? "Value" : m_actionArgumentLabel.c_str());
        ImGui::InputText("##ActionArgumentInput", m_actionArgumentInput.data(), m_actionArgumentInput.size());
        if (ImGui::Button("Apply"))
        {
            if (m_pendingArgumentAction)
            {
                const std::string argument = m_actionArgumentInput.data();
                m_pendingArgumentAction->setArgument(argument);
                m_pendingArgumentAction->execute(m_pendingActionTarget.string());
                m_pendingArgumentAction.reset();
            }
            clearInputBuffer(m_actionArgumentInput);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            m_pendingArgumentAction.reset();
            clearInputBuffer(m_actionArgumentInput);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void GUIContentBrowser::openSaveAsPopup(const std::filesystem::path& target, const std::string& guid)
{
    if (!m_resourceManager || guid.empty())
        return;

    m_pendingSaveAsGuid = guid;

    std::filesystem::path relativeSuggestion = relativeToRoot(target);
    if (relativeSuggestion.empty())
        relativeSuggestion = target.filename();

    auto stem = relativeSuggestion.stem().string();
    if (stem.empty())
        stem = "resource";
    stem += "_copy";

    std::string extension = relativeSuggestion.extension().string();
    if (extension.empty())
        extension = ".worldbin";

    std::filesystem::path suggestion = relativeSuggestion.parent_path() / (stem + extension);
    setInputBuffer(suggestion.generic_string(), m_saveAsInput);
    ImGui::OpenPopup(kSaveAsPopup);
}

void GUIContentBrowser::renderSaveAsPopup()
{
    if (!ImGui::BeginPopup(kSaveAsPopup))
        return;

    ImGui::TextUnformatted("Save Asset As");
    ImGui::TextUnformatted("Path relative to project resources:");
    ImGui::InputText("##SaveAsInput", m_saveAsInput.data(), m_saveAsInput.size());

    if (ImGui::Button("Save"))
    {
        const std::string pathStr = m_saveAsInput.data();
        if (m_resourceManager && !m_pendingSaveAsGuid.empty() && !pathStr.empty())
        {
            std::filesystem::path relPath = std::filesystem::path(pathStr);
            std::filesystem::path absPath = relPath;
            if (relPath.is_absolute())
            {
                std::error_code relEc;
                auto relative = std::filesystem::relative(absPath, m_rootPath, relEc);
                if (!relEc)
                    relPath = relative;
                else
                    relPath = absPath.filename();
            }
            else
            {
                absPath = std::filesystem::path(m_rootPath) / relPath;
            }

            ResourceModule::ResourceSaveParams params;
            params.sourcePathOverride = relPath;
            params.originOverride     = ResourceModule::AssetOrigin::Project;

            auto result =
                m_resourceManager->saveAs(ResourceModule::AssetID(m_pendingSaveAsGuid), absPath, params);
            if (result)
            {
                LT_LOG(LogVerbosity::Info, "ContentBrowser", ("Saved asset copy: " + absPath.string()).c_str());
                m_directoryCache.invalidate(absPath.parent_path());
                m_treeDirty = true;
            }
            else
            {
                LT_LOG(LogVerbosity::Error, "ContentBrowser", "Failed to save asset copy");
            }
        }
        m_pendingSaveAsGuid.clear();
        clearInputBuffer(m_saveAsInput);
        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        m_pendingSaveAsGuid.clear();
        clearInputBuffer(m_saveAsInput);
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void GUIContentBrowser::loadFavorites()
{
    m_favoriteFolders.clear();
    if (!Fs::exists(m_editorSettingsPath.string()))
        return;

    try
    {
        auto jsonData = nlohmann::json::parse(Fs::readTextFile(m_editorSettingsPath.string()));
        if (jsonData.contains("favoriteFolders"))
        {
            for (const auto& entry : jsonData["favoriteFolders"])
                m_favoriteFolders.emplace_back(entry.get<std::string>());
        }
    }
    catch (const std::exception& e)
    {
        const std::string message = std::string("Failed to load favorites: ") + e.what();
        LT_LOG(LogVerbosity::Error, "ContentBrowser", message.c_str());
    }
}

void GUIContentBrowser::saveFavorites() const
{
    nlohmann::json jsonData;
    jsonData["favoriteFolders"] = nlohmann::json::array();
    for (const auto& favorite : m_favoriteFolders)
        jsonData["favoriteFolders"].push_back(favorite.string());

    Fs::writeTextFile(m_editorSettingsPath.string(), jsonData.dump(2));
}

bool GUIContentBrowser::isFavorite(const std::filesystem::path& path) const
{
    const auto canonicalTarget = canonicalString(path);
    return std::any_of(m_favoriteFolders.begin(), m_favoriteFolders.end(),
                       [&](const std::filesystem::path& fav) {
                           return canonicalString(fav) == canonicalTarget;
                       });
}

void GUIContentBrowser::toggleFavorite(const std::filesystem::path& path)
{
    const auto canonicalTarget = canonicalString(path);
    auto it = std::find_if(m_favoriteFolders.begin(), m_favoriteFolders.end(),
                           [&](const std::filesystem::path& fav) {
                               return canonicalString(fav) == canonicalTarget;
                           });
    if (it == m_favoriteFolders.end())
        m_favoriteFolders.push_back(path);
    else
        m_favoriteFolders.erase(it);
}
