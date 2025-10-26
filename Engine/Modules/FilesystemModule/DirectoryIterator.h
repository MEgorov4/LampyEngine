#pragma once
#include <optional>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>

#include "FilesystemModule.h"

namespace FilesystemModule
{
    namespace fs = std::filesystem;

    class DirectoryIterator
    {
        FilesystemModule* m_filesystemModule;
        Logger::Logger* m_logger;
        ProjectModule::ProjectModule* m_projectModule;

        std::optional<uint64_t> m_dirLastEditTime;
        fs::path m_rootPath;
        fs::path m_currentPath;

    public:
        using constr = const std::string&;

        DirectoryIterator(FilesystemModule* filesystemModule
                          , const std::string& rootPath
                          , const std::string& currentPath);

        FResult stepIntoRoot();
        FResult stepIntoParent();
        FResult stepIntoFolder(constr folderName);

        std::string getCurrentDirName();
        std::string getCurrentDir();
        std::string getCurrentDirWithAppend(constr appendName);
        std::vector<std::string> getCurrentDirContents(const ContentSearchFilter& filter);

        bool isRootPath();
        bool isCurrentDirChanged();
    };
}
