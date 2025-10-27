#include "Shader.h"

#include <Modules/ShaderCompilerModule/ShaderCompiler.h>

#include "ResourceManager.h"

namespace ResourceModule
{
	RShader::RShader(const std::string& path) : BaseResource(path)
	{
		
		std::string extension = Fs::extension(path);

		std::string binaryResultPath;

		if (extension == ".vert" || extension == ".frag")
		{
			m_shaderInfo.text = Fs::readTextFile(path);

            binaryResultPath  = GCM(ShaderCompiler::ShaderCompiler)->compileShader(path);
		}

		else if (extension == ".spv")
		{
			binaryResultPath = path;
		}
			
		m_shaderInfo.buffer = Fs::readBinaryFile(binaryResultPath);
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
