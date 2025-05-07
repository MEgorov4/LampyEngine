#pragma once

#include "BaseResource.h"
#include <memory>
#include <string>
#include <vector>

#include "../MemoryModule/GarbageAllocator.h"

struct ShaderInfo
{
	std::vector<uint8_t, GarbageAllocator<uint8_t>> buffer;
	size_t fileSize;
};

class RShader : public BaseResource
{
public:
	RShader(const std::string& path);

	std::vector<uint8_t> getBuffer() const;
	const ShaderInfo& getShaderInfo() const
	{
		return shaderInfo;
	}
private:
	ShaderInfo shaderInfo;
};