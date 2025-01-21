#pragma once
#include <memory>
#include <string>

class Shader
{
	std::string m_vertPath;
	std::string m_fragPath;
public:
	Shader(const std::string& vertPath, const std::string& fragPath);
	
	std::string getShaderHash();
	const std::string& getVertPath();
	const std::string& getFragPath();
};