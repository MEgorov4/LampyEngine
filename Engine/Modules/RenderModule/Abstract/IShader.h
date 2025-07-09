#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "ITexture.h"
#include "../../ResourceModule/Shader.h"
namespace RenderModule
{
	struct alignas(16) ShaderUniformBlock
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
	};

	struct alignas(16) ShaderUniformBlock_CameraData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 position;
	};

	struct alignas(16) ShaderUniformBlock_ModelData
	{
		glm::mat4 model;
	};

	struct alignas(16) ShaderUniformBlock_DirectionalLightData
	{
		glm::vec4 lightDirection;
		glm::vec4 lightColor;
		float lightIntensity;
		float padding[3];
	};

	struct alignas(16) ShaderUniformBlock_PointLight
	{
		glm::vec4 position;
		glm::vec4 color;
		float intensity;
		float padding[3];
	};

	struct TextureBinding
	{
		int binding;
		unsigned int type;
		std::string name;
	};
	
	class IShader
	{
	public:
		IShader(const std::shared_ptr<ResourceModule::RShader>& vertShader, const std::shared_ptr<ResourceModule::RShader>& fragShader) {};

		virtual ~IShader() = default;

		virtual void use() = 0;
		virtual void unbind() = 0;
		virtual void setUniformBlock(const ShaderUniformBlock& data) = 0;
		virtual void setUniformCameraData(const ShaderUniformBlock_CameraData& data) = 0;
		virtual void setUniformModelsData(const std::vector<ShaderUniformBlock_ModelData>& data) = 0;
		virtual void setUniformDirectionalLightData(const ShaderUniformBlock_DirectionalLightData& data) = 0;
		virtual void setUniformPointLightsData(const std::vector<ShaderUniformBlock_PointLight>& data) = 0;
		virtual void scanTextureBindings(const std::unordered_map<std::string, int>& bindingMap) = 0;
		virtual void setUniformData(const std::string& blockName, const void* data, size_t dataSize) = 0;
		virtual bool hasUniformBlock(const std::string& blockName) = 0;
		virtual void bindTextures(const std::unordered_map<std::string, TextureHandle>& textures) = 0;
	};
}
