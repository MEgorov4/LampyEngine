#include "OpenGLShader.h"

#include <GL/glew.h>

namespace RenderModule::OpenGL
{
OpenGLShader::OpenGLShader(const std::shared_ptr<ResourceModule::RShader> &vertShader,
                           const std::shared_ptr<ResourceModule::RShader> &fragShader)
    : IShader(vertShader, fragShader)
{
    LT_LOGI("RenderModule::OpenGLShader", "Construct");
    std::string vertCode = vertShader->getShaderInfo().vertexText;
    std::string fragCode = fragShader->getShaderInfo().fragmentText;

    GLuint vertShaderID = createShaderFromGLSL(vertCode, GL_VERTEX_SHADER);
    GLuint fragShaderID = createShaderFromGLSL(fragCode, GL_FRAGMENT_SHADER);

    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertShaderID);
    glAttachShader(m_programID, fragShaderID);
    glLinkProgram(m_programID);

    GLint success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(m_programID, sizeof(infoLog), NULL, infoLog);
        // LOG_ERROR("OpenGLShader: Shader link error: " + std::string(infoLog));
    }

    glDeleteShader(vertShaderID);
    glDeleteShader(fragShaderID);

    scanUniformBlocks();
}

OpenGLShader::~OpenGLShader()
{
    LT_LOGI("RenderModule::OpenGLShader", "Destruct");
    if (m_programID)
    {
        glUseProgram(0);
        glDeleteProgram(m_programID);
    }

    for (auto &[_, ubo] : m_ubos)
    {
        glDeleteBuffers(1, &ubo);
    }
}

GLuint OpenGLShader::createShaderFromSPIRV(const std::vector<uint8_t> spirvCode, GLenum shaderType)
{
    LT_LOGI("RenderModule::OpenGLShader", "createShaderFromSPIRV");
    GLuint shader = glCreateShader(shaderType);

    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvCode.data(), spirvCode.size());
    glSpecializeShader(shader, "main", 0, nullptr, nullptr);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        // LOG_ERROR("OpenGLShader:createShaderFromSPIRV: Shader specialization error: " + std::string(log));
    }

    return shader;
}

unsigned int OpenGLShader::createShaderFromGLSL(const std::string &source, unsigned int shaderType)
{
    LT_LOGI("RenderModule::OpenGLShader", "createShaderFromGLSL");
    GLuint shader = glCreateShader(shaderType);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);

        LT_LOGE("OpenGLShader", std::format("GLSL Shader compile error: {}", log));
    }

    return shader;
}

void OpenGLShader::scanUniformBlocks()
{
    LT_LOGI("RenderModule::OpenGLShader", "scanUniformBlocks");
    GLint numBlocks = 0;
    glGetProgramiv(m_programID, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);

    for (int i = 0; i < numBlocks; i++)
    {
        char blockName[256];
        GLsizei length = 0;
        glGetActiveUniformBlockName(m_programID, i, sizeof(blockName), &length, blockName);

        GLuint blockIndex = glGetUniformBlockIndex(m_programID, blockName);

        if (blockIndex != GL_INVALID_INDEX)
        {
            GLuint bindingPoint = i;
            glUniformBlockBinding(m_programID, blockIndex, bindingPoint);

            m_uniformBlocks[blockName] = bindingPoint;
            LT_LOGI("OpenGLShader:", std::format("Bound uniform:{} -> bindingPoint {}", blockName, bindingPoint));
        }
    }
}

void OpenGLShader::scanTextureBindings(const std::unordered_map<std::string, int> &bindingMap)
{
    LT_LOGI("RenderModule::OpenGLShader", "scanTextureBindings");
    m_textureBindings.clear();

    GLint numUniforms = 0;
    glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &numUniforms);

    for (GLint i = 0; i < numUniforms; i++)
    {
        char name[256];
        GLsizei length = 0;
        GLenum type;
        GLint size;

        glGetActiveUniform(m_programID, i, sizeof(name), &length, &size, &type, name);

        if (type == GL_SAMPLER_2D)
        {
            GLint location = glGetUniformLocation(m_programID, name);
            if (location < 0)
                continue;

            auto it = bindingMap.find(name);
            if (it == bindingMap.end())
            {
                LT_LOGE("OpenGLShader", std::format("No binding found for sampler:{}", name));
                continue;
            }

            int textureUnit = it->second;

            TextureBinding texBinding;
            texBinding.binding = textureUnit;
            texBinding.type = type;
            texBinding.name = name;

            m_textureBindings[textureUnit] = texBinding;

            glUseProgram(m_programID);
            glUniform1i(location, textureUnit);
            glUseProgram(0);

            LT_LOGI("OpenGLShader:", std::format("Bound sampler:{} -> unit {}", name, textureUnit));
        }
    }
}

void OpenGLShader::bindTextures(const std::unordered_map<std::string, TextureHandle> &textures)
{
    use();

    for (const auto &[key, binding] : m_textureBindings)
    {
        auto it = textures.find(binding.name);
        if (it == textures.end())
            continue;

        glActiveTexture(GL_TEXTURE0 + binding.binding);
        glBindTexture(GL_TEXTURE_2D, it->second.id);
    }
}

void OpenGLShader::setUniformData(const std::string &blockName, const void *data, size_t dataSize)
{
    use();
    if (m_uniformBlocks.find(blockName) == m_uniformBlocks.end())
    {
        return;
    }

    GLuint ubo = getOrCreateUBO(blockName, dataSize);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    void *ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, dataSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr)
    {
        memcpy(ptr, data, dataSize);
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool OpenGLShader::hasUniformBlock(const std::string &blockName)
{
    return m_uniformBlocks.find(blockName) != m_uniformBlocks.end();
}

GLuint OpenGLShader::getOrCreateUBO(const std::string &blockName, size_t dataSize)
{
    if (m_ubos.find(blockName) == m_ubos.end())
    {
        GLuint ubo;
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, dataSize, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        GLuint bindingPoint = m_uniformBlocks[blockName];
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);
        m_ubos[blockName] = ubo;
    }
    return m_ubos[blockName];
}

void OpenGLShader::debugPrintUBO(const std::string &blockName, size_t dataSize)
{
    if (m_ubos.find(blockName) == m_ubos.end())
        return;

    glBindBuffer(GL_UNIFORM_BUFFER, m_ubos[blockName]);
    void *ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_ONLY);
    if (ptr)
    {
        for (size_t i = 0; i < dataSize / sizeof(glm::mat4); i++)
        {
            glm::mat4 *mat = static_cast<glm::mat4 *>(ptr) + i;
        }
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLShader::use()
{
    glUseProgram(m_programID);

    for (auto &[blockName, bindingPoint] : m_uniformBlocks)
    {
        if (m_ubos.find(blockName) != m_ubos.end())
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_ubos[blockName]);
        }
    }
}

void OpenGLShader::unbind()
{
    glUseProgram(0);
}

void OpenGLShader::setUniformBlock(const ShaderUniformBlock &data)
{
}
} // namespace RenderModule::OpenGL
