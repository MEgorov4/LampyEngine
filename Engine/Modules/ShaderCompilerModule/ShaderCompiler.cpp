#include "ShaderCompiler.h"

#include <boost/process.hpp>

namespace ShaderCompiler
{
namespace bp = boost::process;

void ShaderCompiler::startup()
{
    LT_LOG(LogVerbosity::Info, "ShaderCompiler", "Startup");
}

void ShaderCompiler::shutdown()
{
    LT_LOG(LogVerbosity::Info, "ShaderCompiler", "Shudown");
}

std::string ShaderCompiler::compileShader(const std::string& shaderPath)
{
    if (!Fs::exists(shaderPath))
    {
        LT_LOG(LogVerbosity::Error, "ShaderCompiler", "Invalid absolute path");
        return "";
    }

    std::string extension = Fs::extension(shaderPath);
    if (extension != ".frag" && extension != ".vert")
    {
        LT_LOG(LogVerbosity::Error, "ShaderCompiler", "Invalid file input extension must be .frag or.vert");
        return "";
    }

    std::string fileName = Fs::fileName(shaderPath);

    std::string fullOuputFilePath = Fs::currentPath() + "/../Resources/Shaders/Precompile/" + fileName + ".spv";

    int code = bp::system("../Resources/Shaders/Compiler/glslc.exe " + shaderPath + " -o " + fullOuputFilePath);

    if (code != 0)
    {
        LT_LOG(LogVerbosity::Error, "ShaderCompiler", "Failed to compile shared");
        return "";
    }

    return fullOuputFilePath;
}

bool ShaderCompiler::isShaderPrecompiled(const std::string& shaderPath)
{
    if (!Fs::exists(shaderPath))
    {
        LT_LOG(LogVerbosity::Error, "ShaderCompiler", "Invalid absolute path");
        return "";
    }

    std::string extension = Fs::extension(shaderPath);

    if (extension != ".frag" && extension != ".vert")
    {
        LT_LOG(LogVerbosity::Error, "ShaderCompiler", "Invalid shader file input");
        return false;
    }

    std::string fileName = Fs::fileName(shaderPath);
    std::string possiblePrecompiledShaderPath =
        Fs::currentPath() + "/../Resources/Shaders/Precompile/" + fileName + ".spv";

    return Fs::exists(possiblePrecompiledShaderPath);
}

} // namespace ShaderCompiler
