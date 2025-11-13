#pragma once

namespace PhysicsModule
{
    class PhysicsContext;

    class PhysicsLocator
    {
        static inline PhysicsContext* s_ctx = nullptr;

    public:
        static void Provide(PhysicsContext* ctx) noexcept { s_ctx = ctx; }
        
        static PhysicsContext& Get() noexcept 
        { 
            LT_ASSERT_MSG(s_ctx, "PhysicsContext is null");
            return *s_ctx; 
        }
        
        static PhysicsContext* TryGet() noexcept 
        { 
            return s_ctx; 
        }
        
        static void Reset() noexcept { s_ctx = nullptr; }
    };
}

