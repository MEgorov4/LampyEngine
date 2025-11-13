#include "SyncFromPhysicsSystem.h"
#include "../PhysicsLocator.h"
#include "../PhysicsContext/PhysicsContext.h"
#include "../Components/RigidBodyComponent.h"
#include "../../ObjectCoreModule/ECS/Components/ECSComponents.h"

namespace PhysicsModule
{
    void SyncFromPhysicsSystem::Register(flecs::world& world)
    {
        // Run in OnUpdate phase
        world.system<PositionComponent, RotationComponent, RigidBodyComponent>()
            .kind(flecs::OnUpdate)
            .each([](flecs::entity e, PositionComponent& pos, RotationComponent& rot, 
                     RigidBodyComponent& rb)
            {
                // Only sync from physics for dynamic bodies
                if (rb.bodyHandle != InvalidBodyHandle && !rb.isKinematic && !rb.isStatic)
                {
                    auto* ctx = PhysicsLocator::TryGet();
                    if (!ctx)
                        return;

                    glm::vec3 position;
                    glm::quat rotation;
                    if (ctx->getBodyTransform(e, position, rotation))
                    {
                        pos.fromGLMVec(position);
                        rot.fromQuat(rotation);
                    }
                }
            });
    }
}

