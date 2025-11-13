#pragma once

#include <flecs.h>

namespace PhysicsModule
{
    class PhysicsRaycastSystem
    {
    public:
        static void Register(flecs::world& world);
    };
}

