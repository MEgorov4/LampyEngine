#include "PhysicsContext.h"
#include "BulletWorld.h"
#include "DebugDrawer.h"
#include "ContactCallback.h"
#include "../../../Foundation/Event/EventBus.h"
#include "../../../Modules/RenderModule/RenderContext.h"
#include "../Utils/PhysicsConverters.h"
#include "../Factory/PhysicsFactory.h"
#include <btBulletDynamicsCommon.h>

namespace PhysicsModule
{
    PhysicsContext::PhysicsContext(RenderModule::RenderContext* renderContext)
    {
        m_world = std::make_unique<BulletWorld>();
        m_drawer = std::make_unique<DebugDrawer>(renderContext);
        m_contactCallback = std::make_unique<ContactCallback>(nullptr);

        m_world->world->setDebugDrawer(m_drawer.get());
    }

    PhysicsContext::~PhysicsContext()
    {
        // Cleanup is handled by BulletWorld destructor
    }

    void PhysicsContext::step(float dt)
    {
        if (!m_world->world)
            return;

        const int maxSubSteps = 10;
        const float fixedTimeStep = 1.0f / 60.0f;
        m_world->world->stepSimulation(dt, maxSubSteps, fixedTimeStep);

        // Don't call debugDrawWorld here - call it in render phase via debugDraw()
        dispatchCollisionEvents();
    }

    void PhysicsContext::debugDraw()
    {
        if (!m_world->world || !m_debugDrawEnabled || !m_drawer)
            return;

        // Ensure debug drawer is set and mode is up to date
        // The drawer should already be set via setDebugDrawEnabled, but ensure mode is current
        btIDebugDraw* drawer = m_world->world->getDebugDrawer();
        if (drawer)
        {
            drawer->setDebugMode(m_drawer->getDebugMode());
        }

        // Call debugDrawWorld to generate debug primitives
        // This should be called in render phase, before flushDebugPrimitives
        m_world->world->debugDrawWorld();
    }

    void PhysicsContext::connectToEventBus(EngineCore::Foundation::EventBus& bus) noexcept
    {
        m_eventBus = &bus;
        m_contactCallback->bus = &bus;
    }

    bool PhysicsContext::raycast(const glm::vec3& from, const glm::vec3& to, RaycastHit& out)
    {
        if (!m_world->world)
        {
            out.hit = false;
            return false;
        }

        btVector3 btFrom = ToBullet(from);
        btVector3 btTo = ToBullet(to);

        btCollisionWorld::ClosestRayResultCallback callback(btFrom, btTo);
        m_world->world->rayTest(btFrom, btTo, callback);

        if (callback.hasHit())
        {
            out.hit = true;
            out.point = FromBullet(callback.m_hitPointWorld);
            out.normal = FromBullet(callback.m_hitNormalWorld);
            // Distance should be calculated along the ray direction
            // ClosestRayResultCallback.m_closestHitFraction gives the fraction along the ray
            out.distance = callback.m_closestHitFraction * glm::length(to - from);

            // Try to get entity from user pointer
            const btCollisionObject* obj = callback.m_collisionObject;
            if (obj && obj->getUserPointer())
            {
                // Assuming user pointer stores flecs::entity*
                // This will be set by the systems when creating bodies
                flecs::entity* entityPtr = static_cast<flecs::entity*>(obj->getUserPointer());
                if (entityPtr)
                {
                    out.entity = *entityPtr;
                }
            }
        }
        else
        {
            out.hit = false;
        }

        return out.hit;
    }

    // Internal structure to store Bullet objects per entity
    struct PhysicsContext::EntityPhysicsData
    {
        std::unique_ptr<btRigidBody> body;
        PhysicsShapeHandle shapeHandle;
        
        EntityPhysicsData(std::unique_ptr<btRigidBody> b, PhysicsShapeHandle sh)
            : body(std::move(b)), shapeHandle(sh) {}
    };

    PhysicsShapeHandle PhysicsContext::createShape(const PhysicsShapeDesc& desc)
    {
        btCollisionShape* shape = PhysicsFactory::CreateShape(desc);
        if (!shape)
            return InvalidShapeHandle;
            
        PhysicsShapeHandle handle = reinterpret_cast<PhysicsShapeHandle>(shape);
        m_shapes[handle] = std::unique_ptr<btCollisionShape>(shape);
        return handle;
    }

    void PhysicsContext::destroyShape(PhysicsShapeHandle handle)
    {
        auto it = m_shapes.find(handle);
        if (it != m_shapes.end())
        {
            m_shapes.erase(it);
        }
    }

    bool PhysicsContext::createBodyForEntity(flecs::entity entity, const RigidBodyDesc& desc, PhysicsBodyHandle& outHandle)
    {
        flecs::entity_t entityId = entity.id();
        if (m_entityBodies.find(entityId) != m_entityBodies.end())
            return false; // Already exists

        // Create shape
        PhysicsShapeHandle shapeHandle = createShape(desc.shape);
        if (shapeHandle == InvalidShapeHandle)
            return false;

        btCollisionShape* shape = getShapeForHandle(shapeHandle);
        if (!shape)
            return false;

        // Create rigid body
        btTransform transform = ToBullet(desc.position, desc.rotation);
        btDefaultMotionState* motionState = new btDefaultMotionState(transform);

        // For dynamic bodies, ensure mass is > 0, otherwise they won't fall
        btScalar mass = 0.0f;
        if (desc.bodyType == RigidBodyType::Dynamic)
        {
            mass = (desc.mass > 0.0f) ? desc.mass : 1.0f; // Default to 1.0f if mass is 0 or negative
        }
        // For static and kinematic, mass is 0
        
        btVector3 inertia(0, 0, 0);
        if (mass > 0.0f)
        {
            shape->calculateLocalInertia(mass, inertia);
        }

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
        auto body = std::make_unique<btRigidBody>(rbInfo);

        // Set body type flags - clear any existing flags first
        body->setCollisionFlags(0);
        
        if (desc.bodyType == RigidBodyType::Kinematic)
        {
            body->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
            body->setActivationState(DISABLE_DEACTIVATION);
        }
        else if (desc.bodyType == RigidBodyType::Static)
        {
            body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
        }
        // For Dynamic bodies, no special flags needed - they are dynamic by default

        // Store entity pointer for raycast/collision callbacks
        body->setUserPointer(new flecs::entity(entity));

        // Get handle before moving body
        btRigidBody* bodyPtr = body.get();
        outHandle = reinterpret_cast<PhysicsBodyHandle>(bodyPtr);

        m_world->world->addRigidBody(bodyPtr);
        
        m_entityBodies[entityId] = std::make_unique<EntityPhysicsData>(std::move(body), shapeHandle);
        
        return true;
    }

    bool PhysicsContext::destroyBodyForEntity(flecs::entity entity)
    {
        flecs::entity_t entityId = entity.id();
        auto it = m_entityBodies.find(entityId);
        if (it == m_entityBodies.end())
            return false;

        auto& data = it->second;
        if (data->body)
        {
            // Clean up user pointer
            if (data->body->getUserPointer())
            {
                delete static_cast<flecs::entity*>(data->body->getUserPointer());
            }

            m_world->world->removeRigidBody(data->body.get());
        }

        m_entityBodies.erase(it);
        return true;
    }

    bool PhysicsContext::updateBodyTransform(flecs::entity entity, const glm::vec3& position, const glm::quat& rotation)
    {
        btRigidBody* body = getBodyForEntity(entity);
        if (!body)
            return false;

        btTransform transform = ToBullet(position, rotation);
        if (body->getMotionState())
        {
            body->getMotionState()->setWorldTransform(transform);
        }
        else
        {
            body->setWorldTransform(transform);
        }
        return true;
    }

    bool PhysicsContext::getBodyTransform(flecs::entity entity, glm::vec3& position, glm::quat& rotation) const
    {
        btRigidBody* body = getBodyForEntity(entity);
        if (!body || !body->getMotionState())
            return false;

        btTransform transform;
        body->getMotionState()->getWorldTransform(transform);
        FromBullet(transform, position, rotation);
        return true;
    }

    btRigidBody* PhysicsContext::getBodyForEntity(flecs::entity entity) const
    {
        flecs::entity_t entityId = entity.id();
        auto it = m_entityBodies.find(entityId);
        if (it == m_entityBodies.end())
            return nullptr;
        return it->second->body.get();
    }

    btCollisionShape* PhysicsContext::getShapeForHandle(PhysicsShapeHandle handle) const
    {
        auto it = m_shapes.find(handle);
        if (it == m_shapes.end())
            return nullptr;
        return it->second.get();
    }

    btDiscreteDynamicsWorld& PhysicsContext::world() noexcept
    {
        return *m_world->world;
    }

    void PhysicsContext::setDebugDrawEnabled(bool enabled)
    {
        m_debugDrawEnabled = enabled;
        if (m_world->world)
        {
            if (enabled && m_drawer)
            {
                m_world->world->setDebugDrawer(m_drawer.get());
            }
            else
            {
                m_world->world->setDebugDrawer(nullptr);
            }
        }
    }

    bool PhysicsContext::isDebugDrawEnabled() const noexcept
    {
        return m_debugDrawEnabled;
    }

    void PhysicsContext::setDebugMode(int debugMode)
    {
        if (m_drawer)
        {
            m_drawer->setDebugMode(debugMode);
        }
    }

    int PhysicsContext::getDebugMode() const
    {
        if (m_drawer)
        {
            return m_drawer->getDebugMode();
        }
        return 0;
    }

    void PhysicsContext::dispatchCollisionEvents()
    {
        // Collision events are dispatched via ContactCallback
        // This method can be extended for additional event processing
    }
}

