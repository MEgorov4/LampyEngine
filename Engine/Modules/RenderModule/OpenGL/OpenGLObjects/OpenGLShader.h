#pragma once 

#include <string>
#include <unordered_map>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "../../../ResourceModule/Shader.h"
#include "../../Abstract/IShader.h"

class OpenGLShader : public IShader
{
	GLuint m_programID;

	GLuint m_cameraUBO;
	GLuint m_modelUBO;
	GLuint m_directionalLightUBO;
	GLuint m_pointLightUBO;

	std::unordered_map<std::string, GLuint> m_uniformBlocks;  
	std::unordered_map<std::string, GLuint> m_ubos;           

	std::unordered_map<std::string, GLuint> m_textureBindings;

public:

	OpenGLShader(const std::shared_ptr<RShader>& vertShader, const std::shared_ptr<RShader>& fragShader);
	~OpenGLShader();

	GLuint getProgramID() const { return m_programID; }
	void use() override;

	void setUniformBlock(const ShaderUniformBlock& data) override;

	void setUniformData(const std::string& blockName, const void* data, size_t dataSize) override;
	bool hasUniformBlock(const std::string& blockName) override;

	void bindTextures(const std::unordered_map<std::string, uint32_t>& textures) override;

	GLuint getOrCreateUBO(const std::string& blockName, size_t dataSize);

	void debugPrintUBO(const std::string& blockName, size_t dataSize);

private:
	GLuint createShaderFromSPIRV(const std::vector<uint8_t> spirvCode, GLenum shaderType);

	void unbind() override;

	void setUniformCameraData(const ShaderUniformBlock_CameraData& data) override {};
	void setUniformModelsData(const std::vector<ShaderUniformBlock_ModelData>& data) override {};
	void setUniformDirectionalLightData(const ShaderUniformBlock_DirectionalLightData& data) override {};
	void setUniformPointLightsData(const std::vector<ShaderUniformBlock_PointLight>& data) override {};

	void scanUniformBlocks();
	void scanTextureBindings();
};
