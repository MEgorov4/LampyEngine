#pragma once

#include "../Utils/PhysicsTypes.h"

namespace PhysicsModule
{
    struct RigidBodyComponent
    {
        PhysicsBodyHandle bodyHandle = InvalidBodyHandle;
        float mass = 1.0f;
        bool isStatic = false;
        bool isKinematic = false;
        bool needsCreation = true; // Flag to indicate body needs to be created
    };
}

