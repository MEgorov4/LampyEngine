#include "PhysicsStepSystem.h"
#include "../PhysicsLocator.h"
#include "../PhysicsContext/PhysicsContext.h"

namespace PhysicsModule
{
    void PhysicsStepSystem::Register(flecs::world& world)
    {
        // Physics step is now called from PhysicsModule::tick() in Application.cpp
        // This system registration is kept for future use if needed
        // The actual step happens in PhysicsModule::tick() method
    }
}

