#pragma once
#include <GL/glew.h>
// Forward declaration to avoid including GL headers everywhere
typedef unsigned int GLenum;

namespace RenderModule
{
enum class DepthFunc
{
    Never = GL_NEVER,
    Less = GL_LESS,
    Equal = GL_EQUAL,
    LessEqual = GL_LEQUAL,
    Greater = GL_GREATER,
    NotEqual = GL_NOTEQUAL,
    GreaterEqual = GL_GEQUAL,
    Always = GL_ALWAYS
};

enum class BlendFunc
{
    Zero = GL_ZERO,
    One = GL_ONE,
    SrcColor = GL_SRC_COLOR,
    OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
    DstColor = GL_DST_COLOR,
    OneMinusDstColor = GL_ONE_MINUS_DST_COLOR,
    SrcAlpha = GL_SRC_ALPHA,
    OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
    DstAlpha = GL_DST_ALPHA,
    OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA
};

enum class CullFace
{
    Front = GL_FRONT,
    Back = GL_BACK,
    FrontAndBack = GL_FRONT_AND_BACK
};

class RenderState
{
  public:
    // Depth testing
    static void enableDepthTest(bool enable = true);
    static void setDepthFunc(DepthFunc func);
    static void setDepthMask(bool write);

    // Blending
    static void enableBlend(bool enable = true);
    static void setBlendFunc(BlendFunc src, BlendFunc dst);
    static void setBlendEquation(unsigned int equation);

    // Face culling
    static void enableCullFace(bool enable = true);
    static void setCullFace(CullFace face);

    // Line width
    static void setLineWidth(float width);

    // Framebuffer operations
    static void blitDepthBuffer(unsigned int sourceFBO, unsigned int destFBO, int width, int height);

    // State stack (push/pop)
    struct StateSnapshot
    {
        bool depthTest;
        DepthFunc depthFunc;
        bool depthWrite;
        bool blend;
        BlendFunc blendSrc, blendDst;
        bool cullFace;
        CullFace cullFaceMode;
        float lineWidth;
    };

    static StateSnapshot saveState();
    static void restoreState(const StateSnapshot &state);
};
} // namespace RenderModule
