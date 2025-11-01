#include "Fs.h"

#include "../Log/LoggerMacro.h"

#include <fstream>
#include <sstream>
#include <system_error>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace EngineCore::Foundation
{
namespace fs = std::filesystem;

bool Fs::exists(const std::string& path) noexcept
{
    std::error_code ec;
    return fs::exists(path, ec);
}

bool Fs::isFile(const std::string& path) noexcept
{
    std::error_code ec;
    return fs::is_regular_file(path, ec);
}

bool Fs::isDirectory(const std::string& path) noexcept
{
    std::error_code ec;
    return fs::is_directory(path, ec);
}

std::string Fs::currentPath()
{
    std::error_code ec;
    auto p = fs::current_path(ec);
    return ec ? std::string() : p.string();
}

std::string Fs::currentEnginePath()
{
    auto exePath = std::filesystem::current_path();
    if (exePath.filename() == "Debug" || exePath.filename() == "Release")
        exePath = exePath.parent_path();
    return exePath.parent_path().string();
}

std::string Fs::absolutePath(const std::string& relative)
{
    std::error_code ec;
    auto p = fs::absolute(relative, ec);
    return ec ? std::string() : p.string();
}

std::string Fs::fileName(const std::string& path)
{
    return fs::path(path).filename().string();
}

std::string Fs::extension(const std::string& path)
{
    if (!exists(path))
        return {};
    return fs::path(path).extension().string();
}

uint64_t Fs::fileSize(const std::string& path)
{
    if (!exists(path))
        return 0;
    std::error_code ec;
    auto s = fs::file_size(path, ec);
    return ec ? 0ull : static_cast<uint64_t>(s);
}

uint64_t Fs::folderModTime(const std::string& folderPath)
{
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fad{};
    if (GetFileAttributesExA(folderPath.c_str(), GetFileExInfoStandard, &fad))
    {
        return (static_cast<uint64_t>(fad.ftLastWriteTime.dwHighDateTime) << 32) | fad.ftLastWriteTime.dwLowDateTime;
    }
    return 0ull;
#else
    struct stat attr{};
    if (stat(folderPath.c_str(), &attr) == 0)
        return static_cast<uint64_t>(attr.st_mtime);
    return 0ull;
#endif
}

std::string Fs::relativeTo(const std::string& path, const std::string& base)
{
    std::error_code ec;
    auto rel = fs::relative(path, base, ec);
    return ec ? std::string() : rel.string();
}

std::string Fs::readTextFile(const std::string& filePath)
{
    std::ifstream f(filePath, std::ios::in);
    if (!f)
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Failed to open text file: " + filePath);
        return {};
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::vector<uint8_t> Fs::readBinaryFile(const std::string& filePath)
{
    std::ifstream f(filePath, std::ios::binary);
    if (!f)
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Failed to open binary file: " + filePath);
        return {};
    }
    f.seekg(0, std::ios::end);
    const std::streamsize size = f.tellg();
    f.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(static_cast<size_t>(size));
    if (size > 0)
        f.read(reinterpret_cast<char*>(data.data()), size);

    return data;
}

FsResult Fs::writeTextFile(const std::string& filePath, const std::string& content)
{
    std::ofstream f(filePath, std::ios::out);
    if (!f)
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Failed to open file for write: " + filePath);
        return FsResult::Undefined;
    }
    f << content;
    return FsResult::Success;
}

FsResult Fs::appendTextFile(const std::string& filePath, const std::string& content)
{
    std::ofstream f(filePath, std::ios::app);
    if (!f)
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Failed to open file for append: " + filePath);
        return FsResult::Undefined;
    }
    f << content << '\n';
    return FsResult::Success;
}

FsResult Fs::writeBinaryFile(const std::string& filePath, const std::vector<uint8_t>& data)
{
    std::ofstream f(filePath, std::ios::binary | std::ios::out);
    if (!f)
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Failed to open file for binary write: " + filePath);
        return FsResult::Undefined;
    }
    if (!data.empty())
        f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    return FsResult::Success;
}

FsResult Fs::appendBinaryFile(const std::string& filePath, const std::vector<uint8_t>& data)
{
    std::ofstream f(filePath, std::ios::binary | std::ios::app);
    if (!f)
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Failed to open file for binary append: " + filePath);
        return FsResult::Undefined;
    }
    if (!data.empty())
        f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    return FsResult::Success;
}

FsResult Fs::deleteFile(const std::string& path)
{
    if (!exists(path))
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Invalid file path: " + path);
        return FsResult::InvalidPath;
    }
    std::error_code ec;
    if (!fs::remove(path, ec))
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Failed to remove file: " + path);
        return FsResult::Undefined;
    }
    LT_LOG(LogVerbosity::Info, "Filesystem", "File deleted: " + path);
    return FsResult::Success;
}

FsResult Fs::duplicateFileInDirectory(const std::string& path)
{
    if (!exists(path))
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Invalid file path: " + path);
        return FsResult::InvalidPath;
    }

    fs::path original(path);
    fs::path dir     = original.parent_path();
    std::string base = original.stem().string();
    std::string ext  = original.extension().string();

    fs::path candidate = dir / (base + ext);
    int idx            = 1;
    while (exists(candidate.string()))
    {
        candidate = dir / (base + "_" + std::to_string(idx) + ext);
        ++idx;
    }

    std::error_code ec;
    fs::copy_file(original, candidate, fs::copy_options::overwrite_existing, ec);
    if (ec)
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Failed to duplicate: " + path);
        return FsResult::Undefined;
    }

    LT_LOG(LogVerbosity::Info, "Filesystem", "Duplicated to: " + candidate.string());
    return FsResult::Success;
}

FsResult Fs::moveFileToDirectory(const std::string& filePath, const std::string& destinationDir)
{
    if (!exists(filePath) || !isDirectory(destinationDir))
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Invalid file or destination path");
        return FsResult::InvalidPath;
    }

    fs::path src(filePath);
    fs::path dst = fs::path(destinationDir) / src.filename();

    std::error_code ec;
    fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
    if (ec)
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Copy failed: " + filePath);
        return FsResult::Undefined;
    }

    ec.clear();
    fs::remove(src, ec);
    if (ec)
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Remove after copy failed: " + src.string());
        return FsResult::Undefined;
    }

    LT_LOG(LogVerbosity::Info, "Filesystem", "Moved to: " + dst.string());
    return FsResult::Success;
}

FsResult Fs::createDirectory(const std::string& dirPath)
{
    std::error_code ec;
    if (exists(dirPath))
        return FsResult::AlreadyExists;
    if (fs::create_directories(dirPath, ec))
        return FsResult::Success;

    LT_LOG(LogVerbosity::Error, "Filesystem", "Failed to create directory: " + dirPath);
    return FsResult::Undefined;
}

FsResult Fs::createEmptyFile(const std::string& dirPath, const std::string& fileName)
{
    if (!isDirectory(dirPath))
        return FsResult::InvalidPath;

    fs::path out = fs::path(dirPath) / fileName;
    std::ofstream f(out);
    if (!f.is_open())
        return FsResult::Undefined;
    return FsResult::Success;
}

std::vector<std::string> Fs::getDirectoryContents(const std::string& dirPath, const SearchFilter& filter)
{
    std::vector<std::string> out;
    if (!isDirectory(dirPath))
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Invalid directory path: " + dirPath);
        return out;
    }

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(dirPath, ec))
    {
        if (ec)
            break;

        const auto& p          = entry.path();
        const bool isReg       = entry.is_regular_file(ec);
        const bool isDir       = !isReg && entry.is_directory(ec);
        const std::string name = p.filename().string();

        if (filter.contentType == DirContentType::Files && !isReg)
            continue;
        if (filter.contentType == DirContentType::Folders && !isDir)
            continue;

        if (filter.fileExtensions && isReg)
        {
            const auto ext = p.extension().string();
            bool match     = false;
            for (const auto& e : *filter.fileExtensions)
            {
                if (ext == e)
                {
                    match = true;
                    break;
                }
            }
            if (!match)
                continue;
        }

        if (filter.substring && name.find(*filter.substring) == std::string::npos)
            continue;

        out.emplace_back(name);
    }
    return out;
}

FsResult Fs::copyAbsolutePathToClipboard(const std::string& path)
{
    // if (!clip::set_text(path)) { // TODO: Transit this shit
    //   LT_LOG(LogVerbosity::Error, "Filesystem",
    //          "Clipboard set failed (abs): " + path);
    //   return FsResult::Undefined;
    // }
    return FsResult::Success;
}

FsResult Fs::copyRelativePathToClipboard(const std::string& path, const std::string& base)
{
    const auto rel = relativeTo(path, base);
    if (rel.empty())
    {
        LT_LOG(LogVerbosity::Error, "Filesystem", "Relative path failed");
        return FsResult::Undefined;
    }
    // if (!clip::set_text(rel)) { TODO: Transit this shit
    //   LT_LOG(LogVerbosity::Error, "Filesystem",
    //         "Clipboard set failed (rel): " + rel);
    // return FsResult::Undefined;
    //}
    return FsResult::Success;
}

} // namespace EngineCore::Foundation
