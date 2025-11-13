#pragma once

#include "../Utils/PhysicsTypes.h"

// Forward declaration
class btRigidBody;

namespace PhysicsModule
{
    class MaterialFactory
    {
    public:
        static void ApplyMaterial(btRigidBody* body, const PhysicsMaterialDesc& material);
    };
}

