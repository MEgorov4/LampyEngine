#pragma once
#include "../Abstract/ITexture.h"

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

// Основная нода графа (аналог старого Pass)
struct RenderGraphPass
{
    std::string name;
    std::vector<std::string> reads;
    std::vector<std::string> writes;
    std::function<void(const std::vector<RenderGraphResource>&, std::vector<RenderGraphResource>&)> execute;
};
} // namespace RenderModule