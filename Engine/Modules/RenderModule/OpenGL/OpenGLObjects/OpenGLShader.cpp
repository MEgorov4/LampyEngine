#include "OpenGLShader.h"
#include "../../../FilesystemModule/FilesystemModule.h"
#include "../../../LoggerModule/Logger.h"

OpenGLShader::OpenGLShader(const std::shared_ptr<RShader>& vertShader, const std::shared_ptr<RShader>& fragShader)
    : IShader(vertShader, fragShader)
{
    std::vector<uint8_t> vertCode = vertShader->getShaderInfo().buffer;
    std::vector<uint8_t> fragCode = fragShader->getShaderInfo().buffer;

    GLuint vertShaderID = createShaderFromSPIRV(vertCode, GL_VERTEX_SHADER);
    GLuint fragShaderID = createShaderFromSPIRV(fragCode, GL_FRAGMENT_SHADER);

    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertShaderID);
    glAttachShader(m_programID, fragShaderID);
    glLinkProgram(m_programID);

    GLint success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programID, sizeof(infoLog), NULL, infoLog);
        LOG_ERROR("OpenGLShader: Shader link error: " + std::string(infoLog));
    }

    glDeleteShader(vertShaderID);
    glDeleteShader(fragShaderID);

    glGenBuffers(1, &m_UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderUniformBlock), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLuint blockIndex = glGetUniformBlockIndex(m_programID, "ShaderData");
    if (blockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(m_programID, blockIndex, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_UBO);
    }
    else {
        LOG_ERROR("OpenGLShader: UBO block 'ShaderData' not found in shader");
    }
}

OpenGLShader::~OpenGLShader()
{
    if (m_programID) {
        glUseProgram(0); 
        glDeleteProgram(m_programID);
    }
    if (m_UBO) {
        glDeleteBuffers(1, &m_UBO);
    }
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
        LOG_ERROR("OpenGLShader:createShaderFromSPIRV: Shader specialization error: " + std::string(log));
    }

    return shader;
}

void OpenGLShader::unbind()
{
    glUseProgram(0);
}

void OpenGLShader::use()
{
    glUseProgram(m_programID);
}

void OpenGLShader::setUniformBlock(const ShaderUniformBlock& data)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);

    void* ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(ShaderUniformBlock), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr)
    {
        memcpy(ptr, &data, sizeof(ShaderUniformBlock));
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }
    else
    {
        LOG_ERROR("Failed to map UBO buffer!");
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
