#include "Shader.h"

#include <fstream>
#include "../FilesystemModule/FilesystemModule.h"


RShader::RShader(const std::string& path)
{
	shaderInfo.buffer = FS.readBinaryFile(path);
}
