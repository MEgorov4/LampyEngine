#include "ContactCallback.h"
#include "../../../Foundation/Event/EventBus.h"
#include "../Events.h"
#include "../Utils/PhysicsConverters.h"
#include <btBulletDynamicsCommon.h>

namespace PhysicsModule
{
    btScalar ContactCallback::addSingleResult(btManifoldPoint& cp,
                                               const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
                                               const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
    {
        if (!bus)
            return 0.0f;

        using namespace Events::Physics;

        // Get collision objects from wrappers
        const btCollisionObject* colObj0 = colObj0Wrap ? colObj0Wrap->getCollisionObject() : nullptr;
        const btCollisionObject* colObj1 = colObj1Wrap ? colObj1Wrap->getCollisionObject() : nullptr;

        // Try to get entities from user pointers
        if (colObj0 && colObj0->getUserPointer())
        {
            flecs::entity* entityPtr = static_cast<flecs::entity*>(colObj0->getUserPointer());
            if (entityPtr)
                entityA = *entityPtr;
        }

        if (colObj1 && colObj1->getUserPointer())
        {
            flecs::entity* entityPtr = static_cast<flecs::entity*>(colObj1->getUserPointer());
            if (entityPtr)
                entityB = *entityPtr;
        }

        PhysicsCollisionEvent event;
        event.a = entityA;
        event.b = entityB;
        event.point = FromBullet(cp.getPositionWorldOnA());
        event.normal = FromBullet(cp.m_normalWorldOnB);
        event.impulse = cp.getAppliedImpulse();

        bus->emit(event);

        return 0.0f;
    }
}

