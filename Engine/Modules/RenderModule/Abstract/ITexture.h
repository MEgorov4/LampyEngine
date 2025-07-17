#pragma once
#include <memory>
#include "../../ResourceModule/Texture.h"

namespace RenderModule
{
    struct TextureHandle
    {
        uint32_t id;
    };

    class ITexture
    {
    public:
        ITexture(const std::shared_ptr<ResourceModule::RTexture>& texture)
        {
        };

        virtual ~ITexture() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual TextureHandle getTextureID() = 0;
    };
}
