#include "BodyFactory.h"
#include "../Utils/PhysicsTypes.h"
#include "../Utils/PhysicsConverters.h"
#include "ShapeFactory.h"
#include <btBulletDynamicsCommon.h>

namespace PhysicsModule
{
    btRigidBody* BodyFactory::Create(const RigidBodyDesc& desc, btDiscreteDynamicsWorld* world)
    {
        btCollisionShape* shape = ShapeFactory::Create(desc.shape);
        if (!shape)
            return nullptr;

        btTransform transform = ToBullet(desc.position, desc.rotation);
        btDefaultMotionState* motionState = new btDefaultMotionState(transform);

        // For kinematic bodies, use 0 mass but set kinematic flag
        // For static bodies, use 0 mass and set static flag
        // For dynamic bodies, use provided mass
        btScalar mass = 0.0f;
        if (desc.bodyType == RigidBodyType::Dynamic)
        {
            mass = desc.mass;
        }
        
        btVector3 inertia(0, 0, 0);
        if (mass > 0.0f)
        {
            shape->calculateLocalInertia(mass, inertia);
        }

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        // Set body type flags AFTER creation
        if (desc.bodyType == RigidBodyType::Kinematic)
        {
            body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            body->setActivationState(DISABLE_DEACTIVATION);
            // Clear static flag if it was set
            body->setCollisionFlags(body->getCollisionFlags() & ~btCollisionObject::CF_STATIC_OBJECT);
        }
        else if (desc.bodyType == RigidBodyType::Static)
        {
            body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
            // Clear kinematic flag if it was set
            body->setCollisionFlags(body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
        }

        if (world)
        {
            world->addRigidBody(body);
        }

        return body;
    }
}

