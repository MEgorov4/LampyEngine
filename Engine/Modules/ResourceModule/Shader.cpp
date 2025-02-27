#include "Shader.h"

#include <fstream>
#include "../FilesystemModule/FilesystemModule.h"
#include "../ShaderCompilerModule/ShaderCompiler.h"


RShader::RShader(const std::string& path) : BaseResource(path)
{
	std::string extension = FS.getFileExtensions(path);

	std::string binaryResultPath;
	//GLSL Case
	if (extension == ".vert" || extension == ".frag")
	{
		binaryResultPath = SH.compileShader(path);
	}
	//SPIR-V Case
	else if (extension == ".spv")
	{
		binaryResultPath = path;
	}

	shaderInfo.buffer = FS.readBinaryFile(binaryResultPath);
}
