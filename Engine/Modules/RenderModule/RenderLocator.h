#pragma once
#include "RenderContext.h"
namespace RenderModule
{
    class RenderLocator
    {
        static inline RenderContext* s_context = nullptr;

    public:
        static void Provide(RenderContext* ctx) { s_context = ctx; }
        static RenderContext* Get() 
        { 
            return s_context; 
        }
        static RenderContext& Ref() 
        { 
            LT_ASSERT_MSG(s_context, "RenderContext is null");
            return *s_context; 
        }
    };
}
