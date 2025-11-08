#pragma once
#include <cstdint>
#include <string>

#include "ITexture.h"

namespace RenderModule
{
    struct FramebufferData
    {
        int height = 1920, width = 1080;
        bool useDepth = true;
        std::string name;
    };

    class IFramebuffer
    {
    public:
        explicit IFramebuffer(const FramebufferData& data)
        {
        }

        virtual ~IFramebuffer() = default;

        virtual void bind() = 0;
        virtual void unbind() = 0;

        virtual void resize(int newWidth, int newHeight) = 0;

        virtual TextureHandle getColorTexture() = 0;
        virtual TextureHandle getDepthTexture() = 0;
    };
}
