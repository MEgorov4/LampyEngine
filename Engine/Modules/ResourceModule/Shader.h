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
		size_t fileSize;
	};

	class RShader : public BaseResource
	{
	public:
		RShader(const std::string& path, std::shared_ptr<FilesystemModule::FilesystemModule> filesystemModule,
			std::shared_ptr<ShaderCompiler::ShaderCompiler> shaderCompiler);

		std::vector<uint8_t> getBuffer() const;
		const ShaderInfo& getShaderInfo() const
		{
			return m_shaderInfo;
		}
	private:
		ShaderInfo m_shaderInfo;
	};
}