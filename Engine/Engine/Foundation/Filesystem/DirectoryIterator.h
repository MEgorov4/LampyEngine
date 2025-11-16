#pragma once

#include "Foundation/Profiler/ProfileAllocator.h"
#include "FsTypes.h"
#include <filesystem>
namespace EngineCore::Foundation
{
namespace fs = std::filesystem;

class DirectoryIterator final
{
    fs::path m_rootPath;
    fs::path m_currentPath;
    std::optional<uint64_t> m_dirLastEditTime; // folder mod time cache

  public:
    DirectoryIterator(const std::string &rootPath, const std::string &currentPath = {});

    FsResult stepIntoRoot();
    FsResult stepIntoParent();
    FsResult stepIntoFolder(const std::string &folderName);
    FsResult navigateTo(const std::string &absolutePath);

    std::string currentDirName() const;
    std::string currentDir() const;
    std::string currentDirAppend(const std::string &appendName) const;
    std::string rootDir() const;
    std::vector<std::string, ProfileAllocator<std::string>> list(const SearchFilter &filter) const;

    bool isRoot() const;
    bool isCurrentDirChanged();
};
} // namespace EngineCore::Foundation
