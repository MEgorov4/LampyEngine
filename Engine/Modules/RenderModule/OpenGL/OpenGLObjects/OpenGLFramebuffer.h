#pragma once


#include "../../Abstract/IFramebuffer.h"
#include "../../Abstract/ITexture.h"

namespace RenderModule::OpenGL
{
    class OpenGLFramebuffer : public IFramebuffer
    {
        unsigned int fbo;
        int m_width, m_height;

        unsigned int m_colorTexture;
        unsigned int m_depthTexture;
        unsigned int rbo;

        bool m_depth;

    public:
        OpenGLFramebuffer(const FramebufferData& data);
        ~OpenGLFramebuffer() override;

        void bind() override;
        void unbind() override;
        void resize(int newWidth, int newHeight) override;

        TextureHandle getColorTexture() override { return {m_colorTexture}; }
        TextureHandle getDepthTexture() override { return {m_depthTexture}; }

        unsigned int getFBO() const
        {
            return fbo;
        }

    private:
        void addColorAttachment(int format = 0x8058, unsigned int type = 0x1401);
        void addDepthAttachment(bool asTexture = true);
    };
}
