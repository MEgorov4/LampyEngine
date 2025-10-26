#include "OpenGLRenderer.h"

#include <format>
#include <glm/gtx/string_cast.hpp>

#include "../../LoggerModule/Logger.h"
#include "../../WindowModule/WindowModule.h"

#include "../../WindowModule/Window.h"
#include "../../ResourceModule/ResourceManager.h"

#include "OpenGLObjects/OpenGLFramebuffer.h"
#include "OpenGLObjects/OpenGLShader.h"
#include "OpenGLObjects/OpenGLMesh.h"
#include "OpenGLObjects/OpenGLMesh2D.h"
#include "OpenGLObjects/OpenGLTexture.h"
#include "../../../EngineContext/CoreGlobal.h"

namespace RenderModule::OpenGL
{
    OpenGLRenderer::OpenGLRenderer() : IRenderer()
    {
        init();
    }

    void OpenGLRenderer::init()
    {
        GCM(Logger::Logger)->log(Logger::LogVerbosity::Info, "Start initialize OpenGL", "RenderModule_OpenGLRenderer");
        glewExperimental = GL_TRUE;
        
        GLenum err = glewInit();
        if (err != GLEW_OK) 
        {
            GCM(Logger::Logger)->log(Logger::LogVerbosity::Error,
                          std::format("Failed to initialize GLEW: {}", reinterpret_cast<const char*>(glewGetErrorString(err))),
                          "RenderModule_OpenGLRenderer");
            throw std::runtime_error("Failed to initialize GLEW");
        }

        if (!GLEW_ARB_gl_spirv || !GLEW_ARB_spirv_extensions)
        {
            throw std::runtime_error("Required SPIR-V OpenGL extensions are not supported!");
        }

        SDL_GL_SetSwapInterval(1);
        GCM(Logger::Logger)->log(Logger::LogVerbosity::Info, "Enable depth test", "RenderModule_OpenGLRenderer");
        glEnable(GL_DEPTH_TEST);
        GCM(Logger::Logger)->log(Logger::LogVerbosity::Info, "Enable cull face", "RenderModule_OpenGLRenderer");
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    void OpenGLRenderer::waitIdle()
    {
        glFinish();
    }
    
    void OpenGLRenderer::debugMessageHandle(const std::string& message) const
    {
        GCM(Logger::Logger)->log(Logger::LogVerbosity::Info, "OpenGL debug message: " + message, "RenderModule_OpenGLRenderer");
    }
}
