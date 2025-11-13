#pragma once

#include <flecs.h>

namespace PhysicsModule
{
    class SyncToPhysicsSystem
    {
    public:
        static void Register(flecs::world& world);
    };
}

