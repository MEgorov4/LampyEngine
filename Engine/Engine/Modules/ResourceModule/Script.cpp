#include "Script.h"

#include "Foundation/Assert/Assert.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

using namespace ResourceModule;

namespace
{
[[nodiscard]] std::string readScriptBinary(const std::filesystem::path &path)
{
    LT_ASSERT_MSG(!path.empty(), "Script resource path cannot be empty");

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
        throw std::runtime_error("Failed to open script resource: " + path.string());

    uint32_t size = 0;
    ifs.read(reinterpret_cast<char *>(&size), sizeof(size));
    if (!ifs)
        throw std::runtime_error("Failed to read script header: " + path.string());

    LT_ASSERT_MSG(size > 0, "Script resource size is zero");

    std::string buffer(size, '\0');
    ifs.read(buffer.data(), size);
    if (static_cast<uint32_t>(ifs.gcount()) != size)
        throw std::runtime_error("Script resource truncated: " + path.string());

    return buffer;
}
} // namespace

RScript::RScript(const std::string &path)
    : BaseResource(path)
{
    LT_ASSERT_MSG(!path.empty(), "Script path cannot be empty");

    std::filesystem::path fsPath(path);
    m_source = readScriptBinary(fsPath);

    LT_LOGI("RScript", "Loaded script resource: " + fsPath.filename().string());
}

