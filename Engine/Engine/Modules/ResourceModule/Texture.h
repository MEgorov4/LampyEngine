#pragma once
#include "BaseResource.h"
#include "Foundation/Memory/ResourceAllocator.h"

#include <EngineMinimal.h>

using EngineCore::Foundation::ResourceAllocator;

namespace ResourceModule
{
struct TextureInfo
{
    std::vector<uint8_t, ResourceAllocator<uint8_t>> pixels;
    int width    = 0;
    int height   = 0;
    int channels = 0;
};

class RTexture : public BaseResource
{
  public:
    explicit RTexture(const std::string& path);
    ~RTexture() noexcept = default;

    const TextureInfo& getInfo() const noexcept
    {
        return m_info;
    }

  private:
    TextureInfo m_info;
};
} // namespace ResourceModule
