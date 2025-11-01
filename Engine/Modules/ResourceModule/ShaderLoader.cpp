#include "ShaderLoader.h"

#include <stddef.h>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "fstream"

std::vector<char> ShaderLoader::readShaderFile(const std::string& filename)
{	
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error(std::string("failed to open file:") + filename + "!");
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}
