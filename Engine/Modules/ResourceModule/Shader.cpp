#include "Shader.h"

#include <fstream>
#include "../FilesystemModule/FilesystemModule.h"
#include "../ShaderCompilerModule/ShaderCompiler.h"

#include "ResourceManager.h"

namespace ResourceModule
{
	RShader::RShader(const std::string& path, FilesystemModule::FilesystemModule* filesystemModule,
			ShaderCompiler::ShaderCompiler* shaderCompiler) : BaseResource(path)
	{
		
		std::string extension = filesystemModule->getFileExtensions(path);

		std::string binaryResultPath;

		if (extension == ".vert" || extension == ".frag")
		{
			m_shaderInfo.text = filesystemModule->readTextFile(path);
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

	std::string RShader::getText() const
	{
		return m_shaderInfo.text;	
	}
}
