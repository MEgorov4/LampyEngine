#pragma once

#include <glm/glm.hpp>

namespace PhysicsModule
{
    struct CharacterControllerComponent
    {
        float radius = 0.5f;
        float height = 2.0f;
        float stepHeight = 0.3f;
        glm::vec3 velocity = glm::vec3(0.0f);
        bool isGrounded = false;
    };
}

