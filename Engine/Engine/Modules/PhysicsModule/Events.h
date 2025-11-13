#pragma once

#include <glm/glm.hpp>
#include <flecs.h>

namespace Events::Physics
{
    struct PhysicsCollisionEvent
    {
        flecs::entity a;
        flecs::entity b;
        glm::vec3 point;
        glm::vec3 normal;
        float impulse;
    };

    struct PhysicsTriggerEvent
    {
        flecs::entity trigger;
        flecs::entity other;
    };

    struct PhysicsSleepEvent
    {
        flecs::entity body;
        bool sleeping;
    };

    struct PhysicsRaycastHitEvent
    {
        flecs::entity hitEntity;
        glm::vec3 point;
        glm::vec3 normal;
    };
}
