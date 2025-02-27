#include "ShaderCompiler.h"
#include <filesystem>
#include <boost/process.hpp>
#include "../FilesystemModule/FilesystemModule.h";
#include "../LoggerModule/Logger.h"

namespace bp = boost::process;

std::string ShaderCompiler::compileShader(const std::string& shaderPath)
{
	if (!FS.isPathExists(shaderPath))
	{
		LOG_ERROR("ShaderCompiler:compileShader: invalid absolute path");
		return "";
	}

	std::string extension = FS.getFileExtensions(shaderPath);
	if (extension != ".frag" && extension != ".vert")
	{
		LOG_ERROR("ShaderCompiler:compileShader: invalid file input extension must be .frag or .vert");
		return "";
	}
	
	std::string fileName = FS.getFileName(shaderPath);

	std::string fullOuputFilePath = FS.getCurrentPath() + "/../Resources/Shaders/Precompile/" + fileName + ".spv";
		
	int code = bp::system("glslc.exe " + shaderPath + " -o " + fullOuputFilePath);
	
	if (code != 0)
	{
		LOG_ERROR("ShaderCompiler:compileShader: failed to compile shader");
		return "";
	}
	
	return fullOuputFilePath;
}

void ShaderCompiler::compileShaders(const std::vector<std::string>& shaderPaths)
{
}

bool ShaderCompiler::isShaderPrecompiled(const std::string& shaderPath)
{
	if (!FS.isPathExists(shaderPath))
	{
		LOG_ERROR("ShaderCompiler:isShaderPrecompiled: invalid absolute path");
		return "";
	}

	std::string extension = FS.getFileExtensions(shaderPath);

	if (extension != ".frag" && extension != ".vert")
	{
		LOG_ERROR("ShaderCompiler:isShaderPrecompiled: invalid shader file input");
		return false;
	}
	
	std::string fileName = FS.getFileName(shaderPath);
	std::string possiblePrecompiledShaderPath = FS.getCurrentPath() + "/../Resources/Shaders/Precompile/" + fileName + ".spv";
	

	return FS.isPathExists(possiblePrecompiledShaderPath);
}

