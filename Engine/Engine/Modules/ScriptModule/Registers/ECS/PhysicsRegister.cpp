#include "PhysicsRegister.h"

#include "ECSBindingUtils.h"

namespace ScriptModule
{
void PhysicsRegister::registerTypes(sol::state& state, sol::environment& env)
{
    state.new_enum(
        "PhysicsShapeType",
        "Box", PhysicsModule::PhysicsShapeType::Box,
        "Sphere", PhysicsModule::PhysicsShapeType::Sphere,
        "Capsule", PhysicsModule::PhysicsShapeType::Capsule,
        "Cylinder", PhysicsModule::PhysicsShapeType::Cylinder,
        "Mesh", PhysicsModule::PhysicsShapeType::Mesh,
        "ConvexHull", PhysicsModule::PhysicsShapeType::ConvexHull);
    sol::table envTable = env;
    envTable["PhysicsShapeType"] = state["PhysicsShapeType"];

    state.new_usertype<PhysicsModule::PhysicsShapeDesc>(
        "PhysicsShapeDesc",
        sol::constructors<PhysicsModule::PhysicsShapeDesc()>(),
        "type", &PhysicsModule::PhysicsShapeDesc::type,
        "size", &PhysicsModule::PhysicsShapeDesc::size,
        "radius", &PhysicsModule::PhysicsShapeDesc::radius,
        "height", &PhysicsModule::PhysicsShapeDesc::height);

    sol::table shapeCtor = state.create_table();
    shapeCtor["new"]      = []() -> PhysicsModule::PhysicsShapeDesc { return PhysicsModule::PhysicsShapeDesc(); };
    envTable["PhysicsShapeDesc"] = shapeCtor;

    state.new_usertype<PhysicsModule::RigidBodyComponent>(
        "RigidBodyComponent",
        sol::constructors<PhysicsModule::RigidBodyComponent()>(),
        "mass", &PhysicsModule::RigidBodyComponent::mass,
        "isStatic", &PhysicsModule::RigidBodyComponent::isStatic,
        "isKinematic", &PhysicsModule::RigidBodyComponent::isKinematic,
        "needsCreation", &PhysicsModule::RigidBodyComponent::needsCreation);

    sol::table rigidCtor = state.create_table();
    rigidCtor["new"]      = []() -> PhysicsModule::RigidBodyComponent { return PhysicsModule::RigidBodyComponent(); };
    envTable["RigidBodyComponent"] = rigidCtor;

    state.new_usertype<PhysicsModule::ColliderComponent>(
        "ColliderComponent",
        sol::constructors<PhysicsModule::ColliderComponent()>(),
        "shapeDesc", &PhysicsModule::ColliderComponent::shapeDesc,
        "isTrigger", &PhysicsModule::ColliderComponent::isTrigger,
        "needsCreation", &PhysicsModule::ColliderComponent::needsCreation);

    sol::table colliderCtor = state.create_table();
    colliderCtor["new"]      = []() -> PhysicsModule::ColliderComponent { return PhysicsModule::ColliderComponent(); };
    envTable["ColliderComponent"] = colliderCtor;
}
} // namespace ScriptModule

