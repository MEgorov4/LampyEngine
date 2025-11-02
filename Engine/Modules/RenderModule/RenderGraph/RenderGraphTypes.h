#pragma once
#include "../Abstract/ITexture.h"
#include "Foundation/Profiler/ProfileAllocator.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

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
    std::vector<std::string, ProfileAllocator<std::string>> reads;
    std::vector<std::string, ProfileAllocator<std::string>> writes;
    std::function<void(const std::vector<RenderGraphResource, ProfileAllocator<RenderGraphResource>>&,
                       std::vector<RenderGraphResource, ProfileAllocator<RenderGraphResource>>&)>
        execute;
};
} // namespace RenderModule
