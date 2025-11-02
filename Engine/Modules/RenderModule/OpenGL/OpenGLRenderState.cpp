#include "../Abstract/RenderState.h"
#include <GL/glew.h>

namespace RenderModule
{
    void RenderState::enableDepthTest(bool enable)
    {
        if (enable)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    void RenderState::setDepthFunc(DepthFunc func)
    {
        glDepthFunc(static_cast<GLenum>(func));
    }

    void RenderState::setDepthMask(bool write)
    {
        glDepthMask(write ? GL_TRUE : GL_FALSE);
    }

    void RenderState::enableBlend(bool enable)
    {
        if (enable)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
    }

    void RenderState::setBlendFunc(BlendFunc src, BlendFunc dst)
    {
        glBlendFunc(static_cast<GLenum>(src), static_cast<GLenum>(dst));
    }

    void RenderState::setBlendEquation(unsigned int equation)
    {
        glBlendEquation(static_cast<GLenum>(equation));
    }

    void RenderState::enableCullFace(bool enable)
    {
        if (enable)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
    }

    void RenderState::setCullFace(CullFace face)
    {
        glCullFace(static_cast<GLenum>(face));
    }

    void RenderState::setLineWidth(float width)
    {
        glLineWidth(width);
    }

    void RenderState::blitDepthBuffer(unsigned int sourceFBO, unsigned int destFBO, 
                                     int width, int height)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFBO);
        
        glBlitFramebuffer(0, 0, width, height, 
                         0, 0, width, height, 
                         GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFBO);
    }

    RenderState::StateSnapshot RenderState::saveState()
    {
        StateSnapshot snapshot;
        
        snapshot.depthTest = glIsEnabled(GL_DEPTH_TEST);
        GLint depthFunc;
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
        snapshot.depthFunc = static_cast<DepthFunc>(depthFunc);
        GLboolean depthWrite;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
        snapshot.depthWrite = depthWrite == GL_TRUE;
        
        snapshot.blend = glIsEnabled(GL_BLEND);
        GLint blendSrc, blendDst;
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);
        snapshot.blendSrc = static_cast<BlendFunc>(blendSrc);
        snapshot.blendDst = static_cast<BlendFunc>(blendDst);
        
        snapshot.cullFace = glIsEnabled(GL_CULL_FACE);
        GLint cullFaceMode;
        glGetIntegerv(GL_CULL_FACE_MODE, &cullFaceMode);
        snapshot.cullFaceMode = static_cast<CullFace>(cullFaceMode);
        
        glGetFloatv(GL_LINE_WIDTH, &snapshot.lineWidth);
        
        return snapshot;
    }

    void RenderState::restoreState(const StateSnapshot& state)
    {
        enableDepthTest(state.depthTest);
        setDepthFunc(state.depthFunc);
        setDepthMask(state.depthWrite);
        
        enableBlend(state.blend);
        setBlendFunc(state.blendSrc, state.blendDst);
        
        enableCullFace(state.cullFace);
        setCullFace(state.cullFaceMode);
        
        setLineWidth(state.lineWidth);
    }
}

