#include "Shader.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

using namespace ResourceModule;

static std::string readText(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open shader: " + path.string());
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

RShader::RShader(const std::string& path)
    : BaseResource(path)
{
    std::filesystem::path basePath(path);
    std::string stem = basePath.stem().string();
    std::filesystem::path dir = basePath.parent_path();

    std::filesystem::path vertPath = dir / (stem + ".vert");
    std::filesystem::path fragPath = dir / (stem + ".frag");

    if (std::filesystem::exists(vertPath))
    {
        m_info.vertexText = readText(vertPath);
        m_info.totalSize += m_info.vertexText.size();
    }

    if (std::filesystem::exists(fragPath))
    {
        m_info.fragmentText = readText(fragPath);
        m_info.totalSize += m_info.fragmentText.size();
    }

    if (m_info.vertexText.empty() && m_info.fragmentText.empty())
        throw std::runtime_error("Shader source pair not found for: " + path);

    LT_LOGI("RShader", "Loaded GLSL shader pair for: " + stem);
}
