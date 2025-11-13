#include "MaterialFactory.h"
#include "../Utils/PhysicsTypes.h"
#include <btBulletDynamicsCommon.h>

namespace PhysicsModule
{
    void MaterialFactory::ApplyMaterial(btRigidBody* body, const PhysicsMaterialDesc& material)
    {
        if (!body)
            return;

        body->setFriction(material.friction);
        body->setRestitution(material.restitution);
    }
}

