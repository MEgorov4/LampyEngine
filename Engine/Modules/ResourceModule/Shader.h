#pragma once

#include "BaseResource.h"
#include <memory>
#include <string>
#include <vector>

struct ShaderInfo
{
	std::vector<char> buffer;
	size_t fileSize;
};

class RShader : public BaseResource
{
public:
	RShader(const std::string& path);

	const ShaderInfo& getShaderInfo() const
	{
		return shaderInfo;
	}
private:
	ShaderInfo shaderInfo;
};