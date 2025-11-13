#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <flecs.h>
#include <cstdint>

namespace PhysicsModule
{
    // Handle-based types to avoid exposing Bullet types
    using PhysicsBodyHandle = uintptr_t;
    using PhysicsShapeHandle = uintptr_t;
    static constexpr PhysicsBodyHandle InvalidBodyHandle = 0;
    static constexpr PhysicsShapeHandle InvalidShapeHandle = 0;
    enum class PhysicsShapeType : uint8_t
    {
        Box,
        Sphere,
        Capsule,
        Cylinder,
        Mesh,
        ConvexHull
    };

    enum class RigidBodyType : uint8_t
    {
        Static,
        Dynamic,
        Kinematic
    };

    struct RaycastHit
    {
        flecs::entity entity{};
        glm::vec3 point{0.0f};
        glm::vec3 normal{0.0f};
        float distance = 0.0f;
        bool hit = false;
    };

    struct PhysicsShapeDesc
    {
        PhysicsShapeType type = PhysicsShapeType::Box;
        glm::vec3 size = glm::vec3(1.0f);
        float radius = 0.5f;
        float height = 1.0f;
    };

    struct RigidBodyDesc
    {
        float mass = 1.0f;
        RigidBodyType bodyType = RigidBodyType::Dynamic;
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        PhysicsShapeDesc shape;
    };

    struct PhysicsMaterialDesc
    {
        float friction = 0.5f;
        float restitution = 0.0f;
        float density = 1.0f;
    };
}

