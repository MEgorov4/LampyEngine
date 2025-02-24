#pragma once 

#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "../../../ResourceModule/Shader.h"
#include "../../Abstract/IShader.h"

class OpenGLShader : public IShader
{
	GLuint m_programID;
	GLuint m_UBO;
public:
	OpenGLShader(const std::shared_ptr<RShader>& vertShader, const std::shared_ptr<RShader>& fragShader);
	~OpenGLShader();

	GLuint getProgramID() const { return m_programID; }

	void use() override;
	void setUniformBlock(const ShaderUniformBlock& data) override;

private:	
	GLuint createShaderFromSPIRV(const std::vector<uint8_t> spirvCode, GLenum shaderType);
};