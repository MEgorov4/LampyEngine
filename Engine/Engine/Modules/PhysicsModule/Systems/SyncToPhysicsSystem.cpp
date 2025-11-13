#include "SyncToPhysicsSystem.h"
#include "../PhysicsLocator.h"
#include "../PhysicsContext/PhysicsContext.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/ColliderComponent.h"
#include "../../ObjectCoreModule/ECS/Components/ECSComponents.h"
#include "../Utils/PhysicsTypes.h"

namespace PhysicsModule
{
    void SyncToPhysicsSystem::Register(flecs::world& world)
    {
        // Run in OnUpdate phase
        world.system<PositionComponent, RotationComponent, RigidBodyComponent, ColliderComponent>()
            .kind(flecs::OnUpdate)
            .each([](flecs::entity e, PositionComponent& pos, RotationComponent& rot, 
                     RigidBodyComponent& rb, ColliderComponent& collider)
            {
                auto* ctx = PhysicsLocator::TryGet();
                if (!ctx)
                    return;

                // Create body if needed
                if (rb.needsCreation || rb.bodyHandle == InvalidBodyHandle)
                {
                    RigidBodyDesc desc;
                    desc.mass = rb.mass;
                    desc.bodyType = rb.isStatic ? RigidBodyType::Static : 
                                   (rb.isKinematic ? RigidBodyType::Kinematic : RigidBodyType::Dynamic);
                    desc.position = pos.toGLMVec();
                    desc.rotation = rot.toQuat();
                    desc.shape = collider.shapeDesc;
                    
                    // Ensure shape has valid default values if not set
                    if (desc.shape.size == glm::vec3(0.0f))
                    {
                        desc.shape.size = glm::vec3(1.0f);
                    }

                    PhysicsBodyHandle bodyHandle = InvalidBodyHandle;
                    if (ctx->createBodyForEntity(e, desc, bodyHandle))
                    {
                        rb.bodyHandle = bodyHandle;
                        rb.needsCreation = false;
                        collider.needsCreation = false;
                    }
                }
                else if (rb.isKinematic)
                {
                    // Update transform for kinematic bodies
                    glm::vec3 position = pos.toGLMVec();
                    glm::quat rotation = rot.toQuat();
                    ctx->updateBodyTransform(e, position, rotation);
                }
            });
    }
}

