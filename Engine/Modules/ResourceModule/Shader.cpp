#include "Shader.h"

#include <fstream>


RShader::RShader(const std::string& path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file - " + path + std::string("!"));
	}

	shaderInfo.fileSize = (size_t)file.tellg();
	shaderInfo.buffer = std::vector<char>(shaderInfo.fileSize);

	file.seekg(0);
	file.read(shaderInfo.buffer.data(), shaderInfo.fileSize);

	file.close();
}
