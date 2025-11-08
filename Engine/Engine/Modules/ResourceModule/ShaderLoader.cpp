#include "ShaderLoader.h"
#include "Foundation/Assert/Assert.h"

#include "Foundation/Profiler/ProfileAllocator.h"
#include "fstream"

#include <iosfwd>
#include <stddef.h>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

std::vector<char, ProfileAllocator<char>> ShaderLoader::readShaderFile(const std::string& filename)
{
    LT_ASSERT_MSG(!filename.empty(), "Shader filename cannot be empty");
    
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error(std::string("failed to open file:") + filename + "!");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    LT_ASSERT_MSG(fileSize > 0, "Shader file is empty");
    LT_ASSERT_MSG(fileSize < 10 * 1024 * 1024, "Shader file is unreasonably large"); // 10MB limit
    
    std::vector<char, ProfileAllocator<char>> buffer(fileSize);
    
    LT_ASSERT_MSG(buffer.size() == fileSize, "Buffer size mismatch");

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    LT_ASSERT_MSG(!buffer.empty(), "Shader buffer is empty after read");

    return buffer;
}
