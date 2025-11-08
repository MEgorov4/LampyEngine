#include "OpenGLRenderer.h"

#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/WindowModule/Window.h>
#include <Modules/WindowModule/WindowModule.h>
#include "../RenderLocator.h"

#include "OpenGLObjects/OpenGLFramebuffer.h"
#include "OpenGLObjects/OpenGLMesh.h"
#include "OpenGLObjects/OpenGLMesh2D.h"
#include "OpenGLObjects/OpenGLShader.h"
#include "OpenGLObjects/OpenGLTexture.h"

namespace RenderModule::OpenGL
{
OpenGLRenderer::OpenGLRenderer() : IRenderer()
{
    init();
}

void OpenGLRenderer::init()
{
    ZoneScopedN("OpenGLRenderer::init");
    LT_LOGI("RenderModule_OpenGLRenderer", "Start initialize OpenGL");
    glewExperimental = GL_TRUE;

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        LT_LOGE("RenderModule_OpenGLRenderer",
                std::format("Failed to initialize GLEW: {}", reinterpret_cast<const char *>(glewGetErrorString(err))));
        throw std::runtime_error("Failed to initialize GLEW");
    }

    if (!GLEW_ARB_gl_spirv || !GLEW_ARB_spirv_extensions)
    {
        throw std::runtime_error("Required SPIR-V OpenGL extensions are not supported!");
    }

    // Initialize Tracy GPU profiler after glewInit() and when GL context is current
    auto* ctx = RenderLocator::Get();
    if (ctx)
    {
        ctx->initAfterGL();
    }

    SDL_GL_SetSwapInterval(1);
    LT_LOGI("RenderModule_OpenGLRenderer", "Enable depth test");
    glEnable(GL_DEPTH_TEST);
    LT_LOGI("RenderModule_OpenGLRenderer", "Enable cull face");
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void OpenGLRenderer::waitIdle()
{
    glFinish();
}

void OpenGLRenderer::debugMessageHandle(const std::string &message) const
{
    LT_LOGI("RenderModule_OpenGLRenderer", "OpenGL debug message: " + message);
}
} // namespace RenderModule::OpenGL
