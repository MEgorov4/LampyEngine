#pragma once

#include <EngineMinimal.h>

namespace ShaderCompiler
{
	class ShaderCompiler : public IModule
	{
	public:
		void startup() override;
		void shutdown() override;

		std::string compileShader(const std::string& shaderPath);

		bool isShaderPrecompiled(const std::string& shaderPath);
	};
}
