#pragma once
#include <EngineMinimal.h>
#include "Foundation/Memory/ResourceAllocator.h"
#include <string>
#include <vector>
#include <xstring>

using EngineCore::Foundation::ResourceAllocator;

class ShaderLoader
{
    ShaderLoader() {};

  public:
    static std::vector<char, ResourceAllocator<char>> readShaderFile(const std::string& filename);
};
