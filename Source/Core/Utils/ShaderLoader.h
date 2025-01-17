#pragma once
#include <vector>
#include <string>
static class ShaderLoader
{
	ShaderLoader() {};
public:
	static std::vector<char> readShaderFile(const std::string& filename);
};