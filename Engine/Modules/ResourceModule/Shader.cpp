#include "Shader.h"

#include <fstream>
#include "../FilesystemModule/FilesystemModule.h"
#include "../ShaderCompilerModule/ShaderCompiler.h"

#include "../MemoryModule/DoubleStackAllocator.h"
#include "ResourceManager.h"

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

	DoubleStackAllocator* allocator = ResourceManager::getInstance().getDoubleStackAllocator();
	std::vector<uint8_t> temp = FS.readBinaryFile(binaryResultPath);
	shaderInfo.buffer = std::vector<uint8_t, GarbageAllocator<uint8_t>>(temp.begin(), temp.end(), GarbageAllocator<uint8_t>(allocator));
}

std::vector<uint8_t> RShader::getBuffer() const
{
	return std::vector<uint8_t>(shaderInfo.buffer.begin(), shaderInfo.buffer.end());
}
