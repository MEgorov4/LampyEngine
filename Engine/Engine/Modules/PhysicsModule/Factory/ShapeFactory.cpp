#include "ShapeFactory.h"
#include "../Utils/PhysicsTypes.h"
#include "../Utils/PhysicsConverters.h"
#include <btBulletDynamicsCommon.h>

namespace PhysicsModule
{
    btCollisionShape* ShapeFactory::Create(const PhysicsShapeDesc& desc)
    {
        switch (desc.type)
        {
        case PhysicsShapeType::Box:
        {
            btVector3 halfExtents = ToBullet(desc.size * 0.5f);
            return new btBoxShape(halfExtents);
        }
        case PhysicsShapeType::Sphere:
        {
            return new btSphereShape(desc.radius);
        }
        case PhysicsShapeType::Capsule:
        {
            return new btCapsuleShape(desc.radius, desc.height);
        }
        case PhysicsShapeType::Cylinder:
        {
            btVector3 halfExtents = ToBullet(desc.size * 0.5f);
            return new btCylinderShape(halfExtents);
        }
        default:
            // Default to box
            btVector3 halfExtents = ToBullet(desc.size * 0.5f);
            return new btBoxShape(halfExtents);
        }
    }
}

