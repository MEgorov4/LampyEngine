#include "OpenGLShader.h"

#include "../../../FilesystemModule/FilesystemModule.h"
#include "../../../LoggerModule/Logger.h"

OpenGLShader::OpenGLShader(const std::string& vertPath, const std::string& fragPath)
{
	std::vector<uint8_t> vertCode = FS.readBinaryFile(vertPath);
	std::vector<uint8_t> fragCode = FS.readBinaryFile(fragPath);

	GLuint vertShader = createShaderFromSPIRV(vertCode, GL_VERTEX_SHADER);
	GLuint fragShader = createShaderFromSPIRV(fragCode, GL_FRAGMENT_SHADER);

    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertShader);
    glAttachShader(m_programID, fragShader);
    glLinkProgram(m_programID);

    GLint success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programID, 512, NULL, infoLog);
        LOG_ERROR("OpenGLShader: Shader link error: " + std::string(infoLog));
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}

OpenGLShader::~OpenGLShader()
{
    glDeleteProgram(m_programID);
}

void OpenGLShader::use() const
{
	glUseProgram(m_programID);
}

GLuint OpenGLShader::createShaderFromSPIRV(const std::vector<uint8_t> spirvCode, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);

    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvCode.data(), spirvCode.size());
    glSpecializeShader(shader, "main", 0, nullptr, nullptr);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        LOG_ERROR("OpenGLShader:createShaderFromSPIRV: Shader compile error: " + std::string(log));
        exit(1);
    }

    return shader;
}

void OpenGLShader::setUniform1i(const std::string& name, int value)
{
    glUniform1i(glGetUniformLocation(m_programID, name.c_str()), value);
}

void OpenGLShader::setUniform1f(const std::string& name, float value)
{
    glUniform1f(glGetUniformLocation(m_programID, name.c_str()), value);
}

void OpenGLShader::setUniform2f(const std::string& name, const glm::vec2& value)
{
    glUniform2f(glGetUniformLocation(m_programID, name.c_str()), value.x, value.y);
}

void OpenGLShader::setUniform3f(const std::string& name, const glm::vec3& value)
{
    glUniform3f(glGetUniformLocation(m_programID, name.c_str()), value.x, value.y, value.z);
}

void OpenGLShader::setUniform4f(const std::string& name, const glm::vec4& value)
{
    glUniform4f(glGetUniformLocation(m_programID, name.c_str()), value.x, value.y, value.z, value.w);
}

void OpenGLShader::setUniformMat3f(const std::string& name, const glm::mat3& value)
{
    glUniformMatrix3fv(glGetUniformLocation(m_programID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void OpenGLShader::setUniformMat4f(const std::string& name, const glm::mat4& value)
{
    glUniformMatrix4fv(glGetUniformLocation(m_programID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}
