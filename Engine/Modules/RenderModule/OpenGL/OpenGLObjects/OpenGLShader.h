#pragma once 

#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

class OpenGLShader
{
	GLuint m_programID;
public:
	OpenGLShader(const std::string& vertPath, const std::string& fragPath);
	~OpenGLShader();

	void use() const;

	GLuint getProgramID() const { return m_programID; }

	void setUniform1i(const std::string& name, int value);
	void setUniform1f(const std::string& name, float value);
	void setUniform2f(const std::string& name, const glm::vec2& value);
	void setUniform3f(const std::string& name, const glm::vec3& value);
	void setUniform4f(const std::string& name, const glm::vec4& value);
	void setUniformMat3f(const std::string& name, const glm::mat3& value);
	void setUniformMat4f(const std::string& name, const glm::mat4& value);

private:	
	GLuint createShaderFromSPIRV(const std::vector<uint8_t> spirvCode, GLenum shaderType);
};