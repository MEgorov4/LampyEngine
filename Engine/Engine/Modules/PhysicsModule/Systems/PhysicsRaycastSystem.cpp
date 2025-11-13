#include "PhysicsRaycastSystem.h"
#include "../PhysicsLocator.h"
#include "../PhysicsContext/PhysicsContext.h"
#include "../Events.h"
#include "../../../Foundation/Event/EventBus.h"
#include <Core/EventHelpers.h>

namespace PhysicsModule
{
    void PhysicsRaycastSystem::Register(flecs::world& world)
    {
        // This system listens for raycast requests via events
        // The actual raycast is performed through PhysicsContext
        // Systems or scripts can emit raycast requests and receive results via events
    }
}

