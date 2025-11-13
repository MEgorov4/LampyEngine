#pragma once

#include <flecs.h>

namespace PhysicsModule
{
    class SyncFromPhysicsSystem
    {
    public:
        static void Register(flecs::world& world);
    };
}

