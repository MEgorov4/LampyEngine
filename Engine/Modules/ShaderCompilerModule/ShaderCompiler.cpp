#include "ShaderCompiler.h"
#include <filesystem>
#include <boost/process.hpp>

#include "../LoggerModule/Logger.h"
#include "../FilesystemModule/FilesystemModule.h"


namespace ShaderCompiler
{

	namespace bp = boost::process;

	void ShaderCompiler::startup(const ModuleRegistry& registry)
	{
		m_logger = std::dynamic_pointer_cast<Logger::Logger>(registry.getModule("Logger"));
		m_filesystemModule = std::dynamic_pointer_cast<FilesystemModule::FilesystemModule>(registry.getModule("FilesystemModule"));

		m_logger->log(Logger::LogVerbosity::Info, "Startup", "ShaderCompiler");
	}

	void ShaderCompiler::shutdown()
	{
		m_logger->log(Logger::LogVerbosity::Info, "Shutdown", "ShaderCompiler");
	}

	std::string ShaderCompiler::compileShader(const std::string& shaderPath)
	{
		
		if (!m_filesystemModule->isPathExists(shaderPath))
		{
			m_logger->log(Logger::LogVerbosity::Error, "Invalid absolute path", "ShaderCompiler");
			return "";
		}

		std::string extension = m_filesystemModule->getFileExtensions(shaderPath);
		if (extension != ".frag" && extension != ".vert")
		{
			m_logger->log(Logger::LogVerbosity::Error, "Invalid file input extension must be .frag or .vert", "ShaderCompiler");
			return "";
		}

		std::string fileName = m_filesystemModule->getFileName(shaderPath);

		std::string fullOuputFilePath = m_filesystemModule->getCurrentPath() + "/../Resources/Shaders/Precompile/" + fileName + ".spv";

		int code = bp::system("../Resources/Shaders/Compiler/glslc.exe " + shaderPath + " -o " + fullOuputFilePath);

		if (code != 0)
		{
			m_logger->log(Logger::LogVerbosity::Error, "Failed to compile shader", "ShaderCompiler");
			return "";
		}

		return fullOuputFilePath;
	}

	void ShaderCompiler::compileShaders(const std::vector<std::string>& shaderPaths)
	{
	}

	bool ShaderCompiler::isShaderPrecompiled(const std::string& shaderPath)
	{
		if (!m_filesystemModule->isPathExists(shaderPath))
		{
			m_logger->log(Logger::LogVerbosity::Error, "Invalid absolute path", "ShaderCompiler");
			return "";
		}

		std::string extension = m_filesystemModule->getFileExtensions(shaderPath);

		if (extension != ".frag" && extension != ".vert")
		{
			m_logger->log(Logger::LogVerbosity::Error, "Invalid shader file input", "ShaderCompiler");
			return false;
		}

		std::string fileName = m_filesystemModule->getFileName(shaderPath);
		std::string possiblePrecompiledShaderPath = m_filesystemModule->getCurrentPath() + "/../Resources/Shaders/Precompile/" + fileName + ".spv";


		return m_filesystemModule->isPathExists(possiblePrecompiledShaderPath);
	}


}
