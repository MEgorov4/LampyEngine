#pragma once
#include <vector>
#include <string>
#include <xstring>

class ShaderLoader
{
	ShaderLoader() {};
public:
	static std::vector<char> readShaderFile(const std::string& filename);
};