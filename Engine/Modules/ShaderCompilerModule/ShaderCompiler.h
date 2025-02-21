#pragma once
#include <string>
#include <vector>
#include <boost/process.hpp>
#include <filesystem>

class ShaderCompiler
{
public:
    static ShaderCompiler& getInstance()
    {
        static ShaderCompiler instance;
        return instance;
    }

    void compileShader(const std::string& shaderPath);
    void compileShaders(const std::vector<std::string>& shaderPaths);

private:
    ShaderCompiler() = default;
    ~ShaderCompiler() = default;
    ShaderCompiler(const ShaderCompiler&) = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;
};

void ShaderCompiler::compileShader(const std::string& shaderPath)
{
    namespace bp = boost::process;
    
    std::filesystem::path shader(shaderPath);
    std::filesystem::path output = shader;
    output.replace_extension(".spv");

    bp::system("glslc", shaderPath, "-o", output.string());
}

void ShaderCompiler::compileShaders(const std::vector<std::string>& shaderPaths)
{
    for (const auto& shader : shaderPaths)
    {
        compileShader(shader);
    }
} 