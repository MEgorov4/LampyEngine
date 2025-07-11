#include "Shader.h"

#include <fstream>
#include "../FilesystemModule/FilesystemModule.h"
#include "../ShaderCompilerModule/ShaderCompiler.h"

#include "ResourceManager.h"

namespace ResourceModule
{
	RShader::RShader(const std::string& path, std::shared_ptr<FilesystemModule::FilesystemModule> filesystemModule,
			std::shared_ptr<ShaderCompiler::ShaderCompiler> shaderCompiler) : BaseResource(path)
	{
		
		std::string extension = filesystemModule->getFileExtensions(path);

		std::string binaryResultPath;

		if (extension == ".vert" || extension == ".frag")
		{
			binaryResultPath = shaderCompiler->compileShader(path);
		}

		else if (extension == ".spv")
		{
			binaryResultPath = path;
		}
			
		m_shaderInfo.buffer = filesystemModule->readBinaryFile(binaryResultPath);
	}

	std::vector<uint8_t> RShader::getBuffer() const
	{
		return std::vector<uint8_t>(m_shaderInfo.buffer.begin(), m_shaderInfo.buffer.end());
	}
}
