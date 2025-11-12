#include "Shader.h"
#include "Foundation/Assert/Assert.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

using namespace ResourceModule;

static bool readText(const std::filesystem::path& path, std::string& out)
{
    LT_ASSERT_MSG(!path.empty(), "Shader file path cannot be empty");
    
    std::ifstream file(path);
    if (!file.is_open())
    {
        LT_LOGE("RShader", "Failed to open shader: " + path.string());
        return false;
    }
    
    std::stringstream buf;
    buf << file.rdbuf();
    out = buf.str();
    
    if (out.empty())
    {
        LT_LOGE("RShader", "Shader file is empty: " + path.string());
        return false;
    }
    return true;
}

RShader::RShader(const std::string& path)
    : BaseResource(path)
{
    LT_ASSERT_MSG(!path.empty(), "Shader path cannot be empty");
    
    // Initialize as empty
    m_info.vertexText.clear();
    m_info.fragmentText.clear();
    m_info.totalSize = 0;
    
    std::filesystem::path basePath(path);
    std::string stem = basePath.stem().string();
    if (stem.empty())
    {
        LT_LOGE("RShader", "Shader stem is empty: " + path);
        return; // Leave resource empty
    }
    
    std::filesystem::path dir = basePath.parent_path();

    std::filesystem::path vertPath = dir / (stem + ".vert");
    std::filesystem::path fragPath = dir / (stem + ".frag");

    if (std::filesystem::exists(vertPath))
    {
        if (!readText(vertPath, m_info.vertexText))
        {
            // Error already logged
            m_info.vertexText.clear();
            m_info.fragmentText.clear();
            m_info.totalSize = 0;
            return; // Leave resource empty
        }
        m_info.totalSize += m_info.vertexText.size();
    }

    if (std::filesystem::exists(fragPath))
    {
        if (!readText(fragPath, m_info.fragmentText))
        {
            // Error already logged, but don't clear vertex shader if it was loaded
            // Fragment shader is optional
            m_info.fragmentText.clear();
            // Don't return - vertex shader might be valid
        }
        else
        {
            m_info.totalSize += m_info.fragmentText.size();
        }
    }

    if (m_info.vertexText.empty() && m_info.fragmentText.empty())
    {
        LT_LOGE("RShader", "Shader source pair not found for: " + path);
        return; // Leave resource empty
    }
    
    if (m_info.totalSize == 0)
    {
        LT_LOGE("RShader", "Total shader size is zero: " + path);
        m_info.vertexText.clear();
        m_info.fragmentText.clear();
        return; // Leave resource empty
    }

    LT_LOGI("RShader", "Loaded GLSL shader pair for: " + stem);
}
