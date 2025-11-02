#include "Shader.h"
#include "../../EngineContext/Foundation/Assert/Assert.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

using namespace ResourceModule;

static std::string readText(const std::filesystem::path& path)
{
    LT_ASSERT_MSG(!path.empty(), "Shader file path cannot be empty");
    
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open shader: " + path.string());
    
    std::stringstream buf;
    buf << file.rdbuf();
    std::string content = buf.str();
    
    LT_ASSERT_MSG(!content.empty(), "Shader file is empty: " + path.string());
    return content;
}

RShader::RShader(const std::string& path)
    : BaseResource(path)
{
    LT_ASSERT_MSG(!path.empty(), "Shader path cannot be empty");
    
    std::filesystem::path basePath(path);
    std::string stem = basePath.stem().string();
    LT_ASSERT_MSG(!stem.empty(), "Shader stem is empty");
    
    std::filesystem::path dir = basePath.parent_path();

    std::filesystem::path vertPath = dir / (stem + ".vert");
    std::filesystem::path fragPath = dir / (stem + ".frag");

    if (std::filesystem::exists(vertPath))
    {
        m_info.vertexText = readText(vertPath);
        LT_ASSERT_MSG(!m_info.vertexText.empty(), "Vertex shader is empty");
        m_info.totalSize += m_info.vertexText.size();
    }

    if (std::filesystem::exists(fragPath))
    {
        m_info.fragmentText = readText(fragPath);
        LT_ASSERT_MSG(!m_info.fragmentText.empty(), "Fragment shader is empty");
        m_info.totalSize += m_info.fragmentText.size();
    }

    if (m_info.vertexText.empty() && m_info.fragmentText.empty())
        throw std::runtime_error("Shader source pair not found for: " + path);
    
    LT_ASSERT_MSG(m_info.totalSize > 0, "Total shader size is zero");

    LT_LOGI("RShader", "Loaded GLSL shader pair for: " + stem);
}
