#pragma once

namespace PhysicsModule
{
    enum class PhysicsDebugFlags : uint32_t
    {
        None = 0,
        Wireframe = 1 << 0,
        AABB = 1 << 1,
        ContactPoints = 1 << 2,
        Constraints = 1 << 3,
        All = Wireframe | AABB | ContactPoints | Constraints
    };

    inline PhysicsDebugFlags operator|(PhysicsDebugFlags a, PhysicsDebugFlags b)
    {
        return static_cast<PhysicsDebugFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline PhysicsDebugFlags operator&(PhysicsDebugFlags a, PhysicsDebugFlags b)
    {
        return static_cast<PhysicsDebugFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }
}

