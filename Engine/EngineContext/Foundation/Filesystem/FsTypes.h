#pragma once
#include "../Profiler/ProfileAllocator.h"
#include <optional>
#include <vector>
namespace EngineCore::Foundation
{

enum class FsResult : uint8_t
{
    Success = 0,
    InvalidPath,
    AlreadyExists,
    Undefined
};

enum class DirContentType : uint8_t
{
    All = 0,
    Files,
    Folders
};

struct SearchFilter
{
    DirContentType contentType = DirContentType::All;
    std::optional<std::vector<std::string, ProfileAllocator<std::string>>> fileExtensions;
    std::optional<std::string> substring;
};

struct FileStat
{
    std::string path;
    uint64_t sizeBytes = 0;
    uint64_t mtime = 0; // platform-dependent ticks/seconds
    bool isDir = false;
};

} // namespace EngineCore::Foundation
