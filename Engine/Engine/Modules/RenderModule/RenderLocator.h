#pragma once
#include "RenderContext.h"
#include <glm/glm.hpp>

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
    
    inline void DebugDrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color = glm::vec3(1.0f, 0.0f, 0.0f))
    {
        auto* ctx = RenderLocator::Get();
        if (ctx)
        {
            DebugLine line;
            line.from = from;
            line.to = to;
            line.color = color;
            ctx->addDebugLine(line);
        }
    }
    
    inline void DebugDrawBox(const glm::vec3& center, const glm::vec3& size, const glm::vec3& color = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        auto* ctx = RenderLocator::Get();
        if (ctx)
        {
            DebugBox box;
            box.center = center;
            box.size = size;
            box.color = color;
            ctx->addDebugBox(box);
        }
    }
    
    inline void DebugDrawSphere(const glm::vec3& center, float radius, const glm::vec3& color = glm::vec3(0.0f, 0.0f, 1.0f))
    {
        auto* ctx = RenderLocator::Get();
        if (ctx)
        {
            DebugSphere sphere;
            sphere.center = center;
            sphere.radius = radius;
            sphere.color = color;
            ctx->addDebugSphere(sphere);
        }
    }
}
