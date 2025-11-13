#include "TestHelpers.h"
#include <Modules/PhysicsModule/PhysicsContext/PhysicsContext.h>
#include <Modules/PhysicsModule/Utils/PhysicsTypes.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>

namespace PhysicsModuleTest::Helpers
{
    flecs::entity CreateGround(flecs::world& world, 
                               PhysicsModule::PhysicsContext& ctx,
                               const glm::vec3& position,
                               const glm::vec3& size)
    {
        auto entity = world.entity("Ground");
        
        PositionComponent pos;
        pos.fromGLMVec(position);
        
        RotationComponent rot;
        rot.x = 0.0f;
        rot.y = 0.0f;
        rot.z = 0.0f;
        
        entity.set<PositionComponent>(pos);
        entity.set<RotationComponent>(rot);
        
        PhysicsModule::RigidBodyDesc desc;
        desc.bodyType = PhysicsModule::RigidBodyType::Static;
        desc.mass = 0.0f;
        desc.position = position;
        desc.rotation = rot.toQuat();
        desc.shape.type = PhysicsModule::PhysicsShapeType::Box;
        desc.shape.size = size;
        
        PhysicsModule::PhysicsBodyHandle handle;
        ctx.createBodyForEntity(entity, desc, handle);
        return entity;
    }
    
    flecs::entity CreateDynamicBox(flecs::world& world,
                                   PhysicsModule::PhysicsContext& ctx,
                                   const glm::vec3& position,
                                   const glm::vec3& size,
                                   float mass)
    {
        auto entity = world.entity();
        
        PositionComponent pos;
        pos.fromGLMVec(position);
        
        RotationComponent rot;
        rot.x = 0.0f;
        rot.y = 0.0f;
        rot.z = 0.0f;
        
        entity.set<PositionComponent>(pos);
        entity.set<RotationComponent>(rot);
        
        PhysicsModule::RigidBodyDesc desc;
        desc.bodyType = PhysicsModule::RigidBodyType::Dynamic;
        desc.mass = mass;
        desc.position = position;
        desc.rotation = rot.toQuat();
        desc.shape.type = PhysicsModule::PhysicsShapeType::Box;
        desc.shape.size = size;
        
        PhysicsModule::PhysicsBodyHandle handle;
        ctx.createBodyForEntity(entity, desc, handle);
        return entity;
    }
    
    flecs::entity CreateStaticBox(flecs::world& world,
                                  PhysicsModule::PhysicsContext& ctx,
                                  const glm::vec3& position,
                                  const glm::vec3& size)
    {
        auto entity = world.entity();
        
        PositionComponent pos;
        pos.fromGLMVec(position);
        
        RotationComponent rot;
        rot.x = 0.0f;
        rot.y = 0.0f;
        rot.z = 0.0f;
        
        entity.set<PositionComponent>(pos);
        entity.set<RotationComponent>(rot);
        
        PhysicsModule::RigidBodyDesc desc;
        desc.bodyType = PhysicsModule::RigidBodyType::Static;
        desc.mass = 0.0f;
        desc.position = position;
        desc.rotation = rot.toQuat();
        desc.shape.type = PhysicsModule::PhysicsShapeType::Box;
        desc.shape.size = size;
        
        PhysicsModule::PhysicsBodyHandle handle;
        ctx.createBodyForEntity(entity, desc, handle);
        return entity;
    }
    
    flecs::entity CreateSphere(flecs::world& world,
                               PhysicsModule::PhysicsContext& ctx,
                               const glm::vec3& position,
                               float radius,
                               PhysicsModule::RigidBodyType bodyType)
    {
        auto entity = world.entity();
        
        PositionComponent pos;
        pos.fromGLMVec(position);
        
        RotationComponent rot;
        rot.x = 0.0f;
        rot.y = 0.0f;
        rot.z = 0.0f;
        
        entity.set<PositionComponent>(pos);
        entity.set<RotationComponent>(rot);
        
        PhysicsModule::RigidBodyDesc desc;
        desc.bodyType = bodyType;
        desc.mass = (bodyType == PhysicsModule::RigidBodyType::Static) ? 0.0f : 1.0f;
        desc.position = position;
        desc.rotation = rot.toQuat();
        desc.shape.type = PhysicsModule::PhysicsShapeType::Sphere;
        desc.shape.radius = radius;
        
        PhysicsModule::PhysicsBodyHandle handle;
        ctx.createBodyForEntity(entity, desc, handle);
        return entity;
    }
}

