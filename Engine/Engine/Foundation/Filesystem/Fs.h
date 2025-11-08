#pragma once
#include "DirectoryIterator.h"
#include "Foundation/Profiler/ProfileAllocator.h"
#include "FsTypes.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace EngineCore::Foundation
{

class Fs final
{
  public:
    static bool exists(const std::string& path) noexcept;
    static bool isFile(const std::string& path) noexcept;
    static bool isDirectory(const std::string& path) noexcept;

    static std::string currentPath();
    static std::string currentEnginePath();
    static std::string absolutePath(const std::string& relative);
    static std::string fileName(const std::string& path);
    static std::string extension(const std::string& path);
    static uint64_t fileSize(const std::string& path);
    static uint64_t folderModTime(const std::string& folderPath);

    static std::string relativeTo(const std::string& path, const std::string& base);

    static std::string readTextFile(const std::string& filePath);
    static std::vector<uint8_t, ProfileAllocator<uint8_t>> readBinaryFile(const std::string& filePath);
    static FsResult writeTextFile(const std::string& filePath, const std::string& content);
    static FsResult appendTextFile(const std::string& filePath, const std::string& content);
    static FsResult writeBinaryFile(const std::string& filePath,
                                    const std::vector<uint8_t, ProfileAllocator<uint8_t>>& data);
    static FsResult appendBinaryFile(const std::string& filePath,
                                     const std::vector<uint8_t, ProfileAllocator<uint8_t>>& data);

    static FsResult deleteFile(const std::string& path);
    static FsResult duplicateFileInDirectory(const std::string& path);
    static FsResult moveFileToDirectory(const std::string& filePath, const std::string& destinationDir);
    static FsResult createDirectory(const std::string& dirPath);
    static FsResult createEmptyFile(const std::string& dirPath, const std::string& fileName);

    static std::vector<std::string, ProfileAllocator<std::string>> getDirectoryContents(const std::string& dirPath,
                                                                                        const SearchFilter& filter);

    static FsResult copyAbsolutePathToClipboard(const std::string& path);
    static FsResult copyRelativePathToClipboard(const std::string& path, const std::string& base);
};
} // namespace EngineCore::Foundation
