#pragma once

#include <string>
#include <memory>

#include "../../EngineContext/IModule.h"


namespace Logger
{
	class Logger;
}
namespace FilesystemModule
{
	class FilesystemModule;
}
namespace ShaderCompiler
{
	class ShaderCompiler : public IModule
	{
		Logger::Logger* m_logger;
		FilesystemModule::FilesystemModule* m_filesystemModule;
	public:

		void startup() override;

		void shutdown() override;

		std::string compileShader(const std::string& shaderPath);

		bool isShaderPrecompiled(const std::string& shaderPath);
	};
}
