#pragma once
#include <EngineMinimal.h>
#include "BaseResource.h"

namespace ShaderCompiler
{
	class ShaderCompiler;
}

namespace ResourceModule
{
	struct ShaderInfo
	{
		std::vector<uint8_t> buffer;
		std::string text;
		size_t fileSize;
	};

	class RShader : public BaseResource
	{
	public:
		RShader(const std::string& path);

		std::vector<uint8_t> getBuffer() const;
		std::string getText() const;
		const ShaderInfo& getShaderInfo() const
		{
			return m_shaderInfo;
		}
	private:
		ShaderInfo m_shaderInfo;
	};
}