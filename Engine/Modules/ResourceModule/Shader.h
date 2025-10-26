#pragma once

#include <memory>
#include <string>
#include <vector>

#include "BaseResource.h"

namespace ShaderCompiler
{
	class ShaderCompiler;
}

namespace FilesystemModule
{
	class FilesystemModule;
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
		RShader(const std::string& path, FilesystemModule::FilesystemModule* filesystemModule,
			ShaderCompiler::ShaderCompiler* shaderCompiler);

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