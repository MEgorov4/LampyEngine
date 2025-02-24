#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "../../ResourceModule/Shader.h"

struct ShaderUniformBlock
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

class IShader
{
public:
	IShader(const std::shared_ptr<RShader>& vertShader, const std::shared_ptr<RShader>& fragShader) {};

	virtual ~IShader() = default;

	virtual void use() = 0;
	virtual void setUniformBlock(const ShaderUniformBlock& data) = 0;
};
