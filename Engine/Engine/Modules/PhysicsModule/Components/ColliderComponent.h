#pragma once

#include "../Utils/PhysicsTypes.h"

namespace PhysicsModule
{
    struct ColliderComponent
    {
        PhysicsShapeHandle shapeHandle = InvalidShapeHandle;
        PhysicsShapeDesc shapeDesc;
        bool isTrigger = false;
        bool needsCreation = true; // Flag to indicate shape needs to be created
    };
}

