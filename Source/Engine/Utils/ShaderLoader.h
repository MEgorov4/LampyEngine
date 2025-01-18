#pragma once
#include <vector>
#include <string>
class ShaderLoader
{
	ShaderLoader() {};
public:
	static std::vector<char> readShaderFile(const std::string& filename);
};