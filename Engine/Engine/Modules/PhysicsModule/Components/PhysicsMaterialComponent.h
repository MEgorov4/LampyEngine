#pragma once

#include "../Utils/PhysicsTypes.h"

namespace PhysicsModule
{
    struct PhysicsMaterialComponent
    {
        float friction = 0.5f;
        float restitution = 0.0f;
        float density = 1.0f;
    };
}

