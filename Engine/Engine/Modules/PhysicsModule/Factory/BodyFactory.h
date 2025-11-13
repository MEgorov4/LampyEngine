#pragma once

#include "../Utils/PhysicsTypes.h"

// Forward declarations
class btRigidBody;
class btDiscreteDynamicsWorld;

namespace PhysicsModule
{
    class BodyFactory
    {
    public:
        static btRigidBody* Create(const RigidBodyDesc& desc, btDiscreteDynamicsWorld* world);
    };
}

