#include "PhysicsFactory.h"
#include "ShapeFactory.h"
#include "BodyFactory.h"
#include "MaterialFactory.h"

namespace PhysicsModule
{
    btCollisionShape* PhysicsFactory::CreateShape(const PhysicsShapeDesc& desc)
    {
        return ShapeFactory::Create(desc);
    }

    btRigidBody* PhysicsFactory::CreateRigidBody(const RigidBodyDesc& desc, btDiscreteDynamicsWorld* world)
    {
        return BodyFactory::Create(desc, world);
    }

    void PhysicsFactory::ApplyMaterial(btRigidBody* body, const PhysicsMaterialDesc& material)
    {
        MaterialFactory::ApplyMaterial(body, material);
    }
}

