#pragma once
#include "../Abstract/ITexture.h"
#include "Foundation/Memory/ResourceAllocator.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

using EngineCore::Foundation::ResourceAllocator;

namespace RenderModule
{
struct RenderGraphResource
{
    std::string name;
    TextureHandle handle{};
    int width  = 0;
    int height = 0;
};

struct RenderGraphPass
{
    std::string name;
    std::vector<std::string, ResourceAllocator<std::string>> reads;
    std::vector<std::string, ResourceAllocator<std::string>> writes;
    std::function<void(const std::vector<RenderGraphResource, ResourceAllocator<RenderGraphResource>>&,
                       std::vector<RenderGraphResource, ResourceAllocator<RenderGraphResource>>&)>
        execute;
};
} // namespace RenderModule
