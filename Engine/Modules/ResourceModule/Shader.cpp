#include "Shader.h"


RShader::RShader(const std::string& vertPath, const std::string& fragPath) : m_vertPath(vertPath), m_fragPath(fragPath)
{
}

std::string RShader::getShaderHash()
{
	return m_vertPath + '|' + m_fragPath;
}

const std::string& RShader::getVertPath()
{
	return m_vertPath;
}

const std::string& RShader::getFragPath()
{
	return m_fragPath;
}
