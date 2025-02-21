#pragma once
#include <string>
#include <vector>

class ShaderCompiler
{
public:
    static ShaderCompiler& getInstance()
    {
        static ShaderCompiler instance;
        return instance;
    }

    std::string compileShader(const std::string& shaderPath);
    void compileShaders(const std::vector<std::string>& shaderPaths);

    bool isShaderPrecompiled(const std::string& shaderPath);

private:
    ShaderCompiler() = default;
    ~ShaderCompiler() = default;
    ShaderCompiler(const ShaderCompiler&) = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;
};

inline ShaderCompiler& SH = ShaderCompiler::getInstance();
