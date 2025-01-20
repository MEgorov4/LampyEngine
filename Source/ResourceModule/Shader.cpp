#include "Shader.h"


Shader::Shader(const std::string& vertPath, const std::string& fragPath) : m_vertPath(vertPath), m_fragPath(fragPath)
{
}

std::string Shader::getShaderHash()
{
	return m_vertPath + '|' + m_fragPath;
}

const std::string& Shader::getVertPath()
{
	return m_vertPath;
}

const std::string& Shader::getFragPath()
{
	return m_fragPath;
}
