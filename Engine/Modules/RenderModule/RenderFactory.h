// RenderFactory.h
#pragma once
#include "Abstract/IRenderFactory.h"
#include "OpenGL/OpenGLRenderFactory.h"
namespace RenderModule
{
class RenderFactory
{
    inline static std::unique_ptr<IRenderFactory> s_instance = nullptr;

  public:
    static void init()
    {
        s_instance = std::make_unique<OpenGL::OpenGLRenderFactory>();
    }

    static IRenderFactory &get()
    {
        LT_ASSERT_MSG(s_instance, "RenderFactory not initialized");
        return *s_instance;
    }

    static void shutdown()
    {
        if (s_instance)
        {
            s_instance->clearCaches();
            s_instance.reset();
        }
    }
};
} // namespace RenderModule
