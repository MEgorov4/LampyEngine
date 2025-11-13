#include "BulletWorld.h"
#include <btBulletDynamicsCommon.h>

namespace PhysicsModule
{
    BulletWorld::BulletWorld()
    {
        m_config = std::make_unique<btDefaultCollisionConfiguration>();
        m_dispatcher = std::make_unique<btCollisionDispatcher>(m_config.get());
        m_broadphase = std::make_unique<btDbvtBroadphase>();
        m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();

        world = new btDiscreteDynamicsWorld(
            m_dispatcher.get(),
            m_broadphase.get(),
            m_solver.get(),
            m_config.get()
        );

        world->setGravity(btVector3(0, -9.81f, 0));
    }

    BulletWorld::~BulletWorld()
    {
        if (world)
        {
            // Remove all collision objects
            int numObjects = world->getNumCollisionObjects();
            for (int i = numObjects - 1; i >= 0; --i)
            {
                btCollisionObject* obj = world->getCollisionObjectArray()[i];
                world->removeCollisionObject(obj);
            }

            delete world;
            world = nullptr;
        }
    }
}

