#pragma once

#include "../Utils/PhysicsTypes.h"

// Forward declarations
class btCollisionShape;
class btRigidBody;
class btDiscreteDynamicsWorld;

namespace PhysicsModule
{
    class PhysicsFactory
    {
    public:
        static btCollisionShape* CreateShape(const PhysicsShapeDesc& desc);
        static btRigidBody* CreateRigidBody(const RigidBodyDesc& desc, btDiscreteDynamicsWorld* world);
        static void ApplyMaterial(btRigidBody* body, const PhysicsMaterialDesc& material);
    };
}

