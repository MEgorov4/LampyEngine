#pragma once
#include "Fs.h" // Lampy::Filesystem::Fs, SearchFilter, FsResult

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace EngineCore::Foundation
{
namespace fs = std::filesystem;

class DirectoryIterator final
{
    fs::path m_rootPath;
    fs::path m_currentPath;
    std::optional<uint64_t> m_dirLastEditTime; // folder mod time cache

  public:
    DirectoryIterator(const std::string& rootPath, const std::string& currentPath = {});

    // Навигация
    FsResult stepIntoRoot();
    FsResult stepIntoParent();
    FsResult stepIntoFolder(const std::string& folderName);

    // Доступ к состоянию
    std::string currentDirName() const;
    std::string currentDir() const;
    std::string currentDirAppend(const std::string& appendName) const;
    std::vector<std::string> list(const SearchFilter& filter) const;

    bool isRoot() const;
    bool isCurrentDirChanged(); // true при первом вызове или при изменении
                                // времени папки
};
} // namespace EngineCore::Foundation
