#pragma once
#include <EngineMinimal.h>
#include <string>
#include <vector>
#include <xstring>

class ShaderLoader
{
    ShaderLoader() {};

  public:
    static std::vector<char, ProfileAllocator<char>> readShaderFile(const std::string& filename);
};
