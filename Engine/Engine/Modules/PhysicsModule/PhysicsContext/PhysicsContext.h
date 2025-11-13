#pragma once

#include "../Utils/PhysicsTypes.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/ColliderComponent.h"
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <flecs.h>

// Forward declarations
class btDiscreteDynamicsWorld;
class btCollisionShape;
class btRigidBody;

namespace RenderModule
{
    class RenderContext;
}

namespace EngineCore::Foundation
{
    class EventBus;
}

namespace PhysicsModule
{
    class BulletWorld;
    class DebugDrawer;
    class ContactCallback;

    class PhysicsContext
    {
    public:
        explicit PhysicsContext(RenderModule::RenderContext* renderContext);
        ~PhysicsContext();

        PhysicsContext(const PhysicsContext&) = delete;
        PhysicsContext& operator=(const PhysicsContext&) = delete;
        PhysicsContext(PhysicsContext&&) = delete;
        PhysicsContext& operator=(PhysicsContext&&) = delete;

        void step(float dt);
        void connectToEventBus(EngineCore::Foundation::EventBus& bus) noexcept;

        // Raycast
        bool raycast(const glm::vec3& from, const glm::vec3& to, RaycastHit& out);

        // Entity-based API (no Bullet types exposed)
        bool createBodyForEntity(flecs::entity entity, const RigidBodyDesc& desc, PhysicsBodyHandle& outHandle);
        bool destroyBodyForEntity(flecs::entity entity);
        bool updateBodyTransform(flecs::entity entity, const glm::vec3& position, const glm::quat& rotation);
        bool getBodyTransform(flecs::entity entity, glm::vec3& position, glm::quat& rotation) const;

        // World access
        btDiscreteDynamicsWorld& world() noexcept;

        // Debug
        void setDebugDrawEnabled(bool enabled);
        bool isDebugDrawEnabled() const noexcept;
        void setDebugMode(int debugMode); // Set Bullet Physics debug mode flags
        int getDebugMode() const; // Get current Bullet Physics debug mode flags
        void debugDraw(); // Call this in render phase to draw debug primitives

    private:
        void dispatchCollisionEvents();
        
        // Internal mapping: entity -> Bullet objects (PIMPL)
        struct EntityPhysicsData;
        // Use entity ID as key (flecs::entity_t is uint64_t and can be hashed)
        std::unordered_map<flecs::entity_t, std::unique_ptr<EntityPhysicsData>> m_entityBodies;
        std::unordered_map<PhysicsShapeHandle, std::unique_ptr<btCollisionShape>> m_shapes;

        std::unique_ptr<BulletWorld> m_world;
        std::unique_ptr<DebugDrawer> m_drawer;
        std::unique_ptr<ContactCallback> m_contactCallback;
        EngineCore::Foundation::EventBus* m_eventBus = nullptr;
        bool m_debugDrawEnabled = false;
        
        // Internal helpers
        btRigidBody* getBodyForEntity(flecs::entity entity) const;
        btCollisionShape* getShapeForHandle(PhysicsShapeHandle handle) const;
        PhysicsShapeHandle createShape(const PhysicsShapeDesc& desc);
        void destroyShape(PhysicsShapeHandle handle);
    };
}

