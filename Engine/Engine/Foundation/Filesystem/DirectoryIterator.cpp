#include "DirectoryIterator.h"

#include "../Log/LoggerMacro.h"

namespace EngineCore::Foundation
{
DirectoryIterator::DirectoryIterator(const std::string &rootPath, const std::string &currentPath)
{
    // root
    if (Fs::exists(rootPath) && Fs::isDirectory(rootPath))
    {
        m_rootPath = fs::path(rootPath);
    }
    else
    {
        LT_LOG(LogVerbosity::Warning, "Filesystem",
               "DirectoryIterator: invalid rootPath '{}', fallback to currentPath()");
        m_rootPath = fs::path(Fs::currentPath());
    }

    // current
    if (!currentPath.empty() && Fs::exists(currentPath) && Fs::isDirectory(currentPath))
        m_currentPath = fs::path(currentPath);
    else
        m_currentPath = m_rootPath;

    m_dirLastEditTime.reset();
}

FsResult DirectoryIterator::stepIntoRoot()
{
    m_currentPath = m_rootPath;
    m_dirLastEditTime = std::nullopt;
    return FsResult::Success;
}

FsResult DirectoryIterator::stepIntoParent()
{
    if (m_currentPath != m_rootPath)
    {
        m_currentPath = m_currentPath.parent_path();
        m_dirLastEditTime = std::nullopt;
        return FsResult::Success;
    }
    LT_LOG(LogVerbosity::Error, "Filesystem", "DirectoryIterator: cannot step into parent (at root)");
    return FsResult::InvalidPath;
}

FsResult DirectoryIterator::stepIntoFolder(const std::string &folderName)
{
    fs::path next = m_currentPath / fs::path(folderName);
    if (!Fs::exists(next.string()) || !Fs::isDirectory(next.string()))
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "DirectoryIterator: invalid folder to step '{}'");
        return FsResult::InvalidPath;
    }

    m_currentPath = next;
    m_dirLastEditTime = std::nullopt;
    return FsResult::Success;
}

std::string DirectoryIterator::currentDirName() const
{
    return m_currentPath.filename().string();
}

std::string DirectoryIterator::currentDir() const
{
    return m_currentPath.string();
}

std::string DirectoryIterator::currentDirAppend(const std::string &appendName) const
{
    return (m_currentPath / fs::path(appendName)).string();
}

std::vector<std::string, ProfileAllocator<std::string>> DirectoryIterator::list(const SearchFilter &filter) const
{
    return Fs::getDirectoryContents(m_currentPath.string(), filter);
}

bool DirectoryIterator::isRoot() const
{
    return m_currentPath == m_rootPath;
}

bool DirectoryIterator::isCurrentDirChanged()
{
    const uint64_t last = Fs::folderModTime(m_currentPath.string());

    if (!m_dirLastEditTime)
    {
        m_dirLastEditTime = last;
        return true;
    }

    if (*m_dirLastEditTime != last || *m_dirLastEditTime == 0)
    {
        m_dirLastEditTime = last;
        return true;
    }
    return false;
}

} // namespace EngineCore::Foundation
