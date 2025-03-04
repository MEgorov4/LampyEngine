#include "OpenGLShader.h"
#include "glm/gtx/string_cast.hpp"
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

	scanUniformBlocks();
	scanTextureBindings();
}

OpenGLShader::~OpenGLShader()
{
	if (m_programID) {
		glUseProgram(0);
		glDeleteProgram(m_programID);
	}

	for (auto& [_, ubo] : m_ubos)
	{
		glDeleteBuffers(1, &ubo);
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

void OpenGLShader::scanUniformBlocks()
{
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
			LOG_INFO("Shader [" + std::to_string(m_programID) + "] found UBO: " + std::string(blockName) + " at binding " + std::to_string(bindingPoint));
		}
	}
}

void OpenGLShader::scanTextureBindings()
{
	GLint numUniforms = 0;
	glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &numUniforms);

	std::unordered_map<std::string, int> expectedBindings = {
		{"shadowMap", 0},
		{"reflectionMap", 1},
		{"lightMap", 2},
		{"customMap", 3},
		{"objectAlbedo", 4},
		{"objectNormal", 5},
		{"objectSpecular", 6},
		{"objectRoughness", 7},
		{"objectEmission", 8}
	};

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
			if (location != -1)
			{
				int textureUnit = expectedBindings.contains(name) ? expectedBindings[name] : location;

				// Записываем в мапу (чтобы потом использовать в bindTextures)
				m_textureBindings[name] = textureUnit;

				// Привязываем текстуру к юниту
				glUseProgram(m_programID);
				glUniform1i(location, textureUnit);
				glUseProgram(0);

				LOG_INFO("Shader [" + std::to_string(m_programID) + "] bound texture: " + std::string(name) + " to unit " + std::to_string(textureUnit));
			}
		}
	}
}



void OpenGLShader::bindTextures(const std::unordered_map<std::string, uint32_t>& textures)
{
	use();

	for (auto& [name, unit] : m_textureBindings)
	{
		if (textures.find(name) != textures.end())
		{
			GLuint textureID = textures.at(name);

			glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(GL_TEXTURE_2D, textureID);
		}
		else
		{
			LOG_WARNING("Texture [" + name + "] not found in provided textures!");
		}
	}
}

void OpenGLShader::setUniformData(const std::string& blockName, const void* data, size_t dataSize)
{
	use();
	if (m_uniformBlocks.find(blockName) == m_uniformBlocks.end())
	{
		LOG_WARNING("UBO [" + blockName + "] not found in shader!");
		return;
	}

	GLuint ubo = getOrCreateUBO(blockName, dataSize);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	void* ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, dataSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	if (ptr)
	{
		memcpy(ptr, data, dataSize);
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
	else
	{
		LOG_ERROR("UBO [" + blockName + "] mapping failed!");
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

bool OpenGLShader::hasUniformBlock(const std::string& blockName)
{
	return m_uniformBlocks.find(blockName) != m_uniformBlocks.end();
}

GLuint OpenGLShader::getOrCreateUBO(const std::string& blockName, size_t dataSize)
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

		LOG_INFO("Created UBO [" + blockName + "] with size: " + std::to_string(dataSize));
	}
	return m_ubos[blockName];
}
void OpenGLShader::debugPrintUBO(const std::string& blockName, size_t dataSize)
{
	if (m_ubos.find(blockName) == m_ubos.end()) return;

	glBindBuffer(GL_UNIFORM_BUFFER, m_ubos[blockName]);
	void* ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_ONLY);
	if (ptr)
	{
		LOG_INFO("UBO [" + blockName + "] content after upload:");
		for (size_t i = 0; i < dataSize / sizeof(glm::mat4); i++)
		{
			glm::mat4* mat = static_cast<glm::mat4*>(ptr) + i;
			LOG_INFO("Model[" + std::to_string(i) + "]:\n" + glm::to_string(*mat));
		}
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void OpenGLShader::use()
{
	glUseProgram(m_programID);

	for (auto& [blockName, bindingPoint] : m_uniformBlocks)
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

void OpenGLShader::setUniformBlock(const ShaderUniformBlock& data)
{

}
