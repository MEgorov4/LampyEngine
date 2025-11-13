#pragma once

#include <btBulletDynamicsCommon.h>
#include <flecs.h>

namespace EngineCore::Foundation
{
    class EventBus;
}

namespace PhysicsModule
{
    struct ContactCallback : public btCollisionWorld::ContactResultCallback
    {
        EngineCore::Foundation::EventBus* bus;
        flecs::entity entityA;
        flecs::entity entityB;

        explicit ContactCallback(EngineCore::Foundation::EventBus* b) : bus(b) {}

        // ContactResultCallback::addSingleResult signature from Bullet
        // Used for contact queries (contactTest, contactPairTest)
        btScalar addSingleResult(btManifoldPoint& cp,
                                 const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
                                 const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override;
    };
}

