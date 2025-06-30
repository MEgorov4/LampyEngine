#pragma once

#include <string>
#include <memory>

#include "../../EngineContext/IModule.h"
#include "../../EngineContext/ModuleRegistry.h"


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
		std::shared_ptr<Logger::Logger> m_logger;
		std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
	public:

		void startup(const ModuleRegistry& registry) override;

		void shutdown() override;

		std::string compileShader(const std::string& shaderPath);
		void compileShaders(const std::vector<std::string>& shaderPaths);

		bool isShaderPrecompiled(const std::string& shaderPath);
	};
}
