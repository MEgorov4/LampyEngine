#pragma once

#include "../Utils/PhysicsTypes.h"

// Forward declaration
class btCollisionShape;

namespace PhysicsModule
{
    class ShapeFactory
    {
    public:
        static btCollisionShape* Create(const PhysicsShapeDesc& desc);
    };
}

