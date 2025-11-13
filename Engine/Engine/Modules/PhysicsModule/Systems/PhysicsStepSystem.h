#pragma once

#include <flecs.h>

namespace PhysicsModule
{
    class PhysicsStepSystem
    {
    public:
        static void Register(flecs::world& world);
    };
}

