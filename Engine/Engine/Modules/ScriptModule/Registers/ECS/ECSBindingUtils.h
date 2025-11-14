#pragma once

#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ObjectCoreModule/ECS/ComponentRegistry.h>
#include <Modules/ObjectCoreModule/ECS/EntityWorld.h>
#include <Modules/PhysicsModule/Components/RigidBodyComponent.h>
#include <Modules/PhysicsModule/Components/ColliderComponent.h>
#include <Modules/PhysicsModule/Utils/PhysicsTypes.h>
#include <Modules/ResourceModule/Asset/AssetID.h>

#include <glm/glm.hpp>
#include <flecs.h>
#include <optional>

namespace ScriptModule
{
template <typename Comp>
inline void RemoveComponentIfPresent(flecs::entity& entity)
{
    if (entity.has<Comp>())
    {
        entity.remove<Comp>();
    }
}
} // namespace ScriptModule

