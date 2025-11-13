#include <gtest/gtest.h>
#include <Modules/PhysicsModule/PhysicsContext/PhysicsContext.h>
#include <Modules/PhysicsModule/Components/RigidBodyComponent.h>
#include <Modules/PhysicsModule/Components/ColliderComponent.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/RenderModule/RenderContext.h>
#include <Foundation/Memory/MemorySystem.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Core/Core.h>
#include <flecs.h>

using namespace PhysicsModule;
using namespace RenderModule;

class EcsSyncTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize MemorySystem before creating RenderContext
        EngineCore::Foundation::MemorySystem::startup(1024 * 1024, 4 * 1024 * 1024); // 1MB frame, 4MB persistent
        
        // Register ResourceManager in CoreLocator for RenderContext
        auto resourceManager = std::make_shared<ResourceModule::ResourceManager>();
        EngineCore::Base::Core::Register(resourceManager, 15);
        
        renderContext = std::make_unique<RenderContext>();
        context = std::make_unique<PhysicsContext>(renderContext.get());
        world = std::make_unique<flecs::world>();
    }
    
    void TearDown() override
    {
        world.reset();
        context.reset();
        renderContext.reset();
        
        // Cleanup CoreLocator
        EngineCore::Base::Core::ShutdownAll();
        
        // Shutdown MemorySystem after cleanup
        EngineCore::Foundation::MemorySystem::shutdown();
    }
    
    std::unique_ptr<RenderContext> renderContext;
    std::unique_ptr<PhysicsContext> context;
    std::unique_ptr<flecs::world> world;
};

// ============================================================================
// Component Creation Tests
// ============================================================================

TEST_F(EcsSyncTest, CreateEntityWithPhysicsComponents)
{
    auto entity = world->entity("PhysicsEntity");
    
    PositionComponent pos;
    pos.x = 0.0f;
    pos.y = 0.0f;
    pos.z = 0.0f;
    
    RotationComponent rot;
    rot.x = 0.0f;
    rot.y = 0.0f;
    rot.z = 0.0f;
    
    RigidBodyComponent rb;
    rb.mass = 1.0f;
    rb.isStatic = false;
    rb.isKinematic = false;
    rb.needsCreation = true;
    
    ColliderComponent collider;
    collider.shapeDesc.type = PhysicsShapeType::Box;
    collider.shapeDesc.size = glm::vec3(1.0f);
    collider.needsCreation = true;
    
    entity.set<PositionComponent>(pos);
    entity.set<RotationComponent>(rot);
    entity.set<RigidBodyComponent>(rb);
    entity.set<ColliderComponent>(collider);
    
    // Verify components are set
    EXPECT_TRUE(entity.has<PositionComponent>());
    EXPECT_TRUE(entity.has<RotationComponent>());
    EXPECT_TRUE(entity.has<RigidBodyComponent>());
    EXPECT_TRUE(entity.has<ColliderComponent>());
}

// ============================================================================
// Body Creation from Components Tests
// ============================================================================

TEST_F(EcsSyncTest, CreateBodyFromComponents)
{
    auto entity = world->entity("BodyEntity");
    
    PositionComponent pos;
    pos.x = 0.0f;
    pos.y = 0.0f;
    pos.z = 0.0f;
    
    RotationComponent rot;
    rot.x = 0.0f;
    rot.y = 0.0f;
    rot.z = 0.0f;
    
    entity.set<PositionComponent>(pos);
    entity.set<RotationComponent>(rot);
    
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Dynamic;
    desc.mass = 1.0f;
    desc.position = pos.toGLMVec();
    desc.rotation = rot.toQuat();
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(1.0f);
    
    PhysicsBodyHandle handle;
    bool created = context->createBodyForEntity(entity, desc, handle);
    EXPECT_TRUE(created);
    EXPECT_NE(handle, InvalidBodyHandle);
    
    // Verify body exists by checking transform
    glm::vec3 position;
    glm::quat rotation;
    bool gotTransform = context->getBodyTransform(entity, position, rotation);
    EXPECT_TRUE(gotTransform);
    
    context->destroyBodyForEntity(entity);
}

// ============================================================================
// Transform Sync Tests
// ============================================================================

TEST_F(EcsSyncTest, SyncTransformToPhysics)
{
    auto entity = world->entity("SyncEntity");
    
    PositionComponent pos;
    pos.x = 5.0f;
    pos.y = 10.0f;
    pos.z = 15.0f;
    
    RotationComponent rot;
    rot.fromEulerDegrees(glm::vec3(0.0f, 45.0f, 0.0f));
    
    entity.set<PositionComponent>(pos);
    entity.set<RotationComponent>(rot);
    
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Kinematic;
    desc.mass = 0.0f;
    desc.position = pos.toGLMVec();
    desc.rotation = rot.toQuat();
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(1.0f);
    
    PhysicsBodyHandle handle;
    context->createBodyForEntity(entity, desc, handle);
    
    // Update transform
    glm::vec3 newPos(20.0f, 30.0f, 40.0f);
    glm::quat newRot = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    bool updated = context->updateBodyTransform(entity, newPos, newRot);
    EXPECT_TRUE(updated);
    
    // Verify transform was updated
    glm::vec3 physicsPos;
    glm::quat physicsRot;
    bool gotTransform = context->getBodyTransform(entity, physicsPos, physicsRot);
    EXPECT_TRUE(gotTransform);
    EXPECT_NEAR(physicsPos.x, 20.0f, 0.1f);
    
    context->destroyBodyForEntity(entity);
}

TEST_F(EcsSyncTest, SyncTransformFromPhysics)
{
    auto entity = world->entity("SyncFromEntity");
    
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Dynamic;
    desc.mass = 1.0f;
    desc.position = glm::vec3(0.0f, 10.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(0.5f);
    
    PhysicsBodyHandle handle;
    context->createBodyForEntity(entity, desc, handle);
    
    // Step simulation (body should fall)
    for (int i = 0; i < 60; ++i)
    {
        context->step(1.0f / 60.0f);
    }
    
    // Get transform from physics
    glm::vec3 physicsPos;
    glm::quat physicsRot;
    bool gotTransform = context->getBodyTransform(entity, physicsPos, physicsRot);
    EXPECT_TRUE(gotTransform);
    
    // Position should have changed (body fell)
    EXPECT_LT(physicsPos.y, 10.0f);
    
    context->destroyBodyForEntity(entity);
}

