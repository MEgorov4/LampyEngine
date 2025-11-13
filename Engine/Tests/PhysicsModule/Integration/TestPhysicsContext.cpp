#include <gtest/gtest.h>
#include <Modules/PhysicsModule/PhysicsContext/PhysicsContext.h>
#include <Modules/PhysicsModule/Utils/PhysicsTypes.h>
#include <Modules/RenderModule/RenderContext.h>
#include <Foundation/Memory/MemorySystem.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Core/Core.h>

using namespace PhysicsModule;
using namespace RenderModule;

class PhysicsContextTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize MemorySystem before creating RenderContext
        EngineCore::Foundation::MemorySystem::startup(1024 * 1024, 4 * 1024 * 1024); // 1MB frame, 4MB persistent
        
        // Register ResourceManager in CoreLocator for RenderContext
        auto resourceManager = std::make_shared<ResourceModule::ResourceManager>();
        EngineCore::Base::Core::Register(resourceManager, 15);
        
        // Create a dummy RenderContext for PhysicsContext
        renderContext = std::make_unique<RenderContext>();
        context = std::make_unique<PhysicsContext>(renderContext.get());
    }
    
    void TearDown() override
    {
        context.reset();
        renderContext.reset();
        
        // Cleanup CoreLocator
        EngineCore::Base::Core::ShutdownAll();
        
        // Shutdown MemorySystem after cleanup
        EngineCore::Foundation::MemorySystem::shutdown();
    }
    
    std::unique_ptr<RenderContext> renderContext;
    std::unique_ptr<PhysicsContext> context;
};

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(PhysicsContextTest, WorldInitialization)
{
    // Context should be initialized with gravity
    EXPECT_NO_THROW(context->step(1.0f / 60.0f));
}

TEST_F(PhysicsContextTest, StepSimulationDoesNotCrash)
{
    EXPECT_NO_THROW(context->step(1.0f / 60.0f));
    EXPECT_NO_THROW(context->step(1.0f / 60.0f));
    EXPECT_NO_THROW(context->step(1.0f / 60.0f));
}

TEST_F(PhysicsContextTest, MultipleSteps)
{
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_NO_THROW(context->step(1.0f / 60.0f));
    }
}

// ============================================================================
// Shape Creation Tests (via createBodyForEntity)
// ============================================================================

TEST_F(PhysicsContextTest, CreateBoxShapeViaBody)
{
    flecs::world world;
    auto entity = world.entity("ShapeTestEntity");
    
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Static;
    desc.mass = 0.0f;
    desc.position = glm::vec3(0.0f, 0.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(1.0f, 1.0f, 1.0f);
    
    PhysicsBodyHandle bodyHandle;
    bool created = context->createBodyForEntity(entity, desc, bodyHandle);
    EXPECT_TRUE(created);
    EXPECT_NE(bodyHandle, InvalidBodyHandle);
    
    context->destroyBodyForEntity(entity);
}

TEST_F(PhysicsContextTest, CreateSphereShapeViaBody)
{
    flecs::world world;
    auto entity = world.entity("SphereTestEntity");
    
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Static;
    desc.mass = 0.0f;
    desc.position = glm::vec3(0.0f, 0.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Sphere;
    desc.shape.radius = 1.0f;
    
    PhysicsBodyHandle bodyHandle;
    bool created = context->createBodyForEntity(entity, desc, bodyHandle);
    EXPECT_TRUE(created);
    EXPECT_NE(bodyHandle, InvalidBodyHandle);
    
    context->destroyBodyForEntity(entity);
}

// ============================================================================
// Body Creation Tests
// ============================================================================

TEST_F(PhysicsContextTest, CreateStaticBodyForEntity)
{
    flecs::world world;
    auto entity = world.entity("TestEntity");
    
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Static;
    desc.mass = 0.0f;
    desc.position = glm::vec3(0.0f, 0.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(1.0f);
    
    PhysicsBodyHandle handle;
    bool created = context->createBodyForEntity(entity, desc, handle);
    EXPECT_TRUE(created);
    EXPECT_NE(handle, InvalidBodyHandle);
    
    // Verify body exists
    glm::vec3 pos;
    glm::quat rot;
    bool gotTransform = context->getBodyTransform(entity, pos, rot);
    EXPECT_TRUE(gotTransform);
    
    context->destroyBodyForEntity(entity);
}

TEST_F(PhysicsContextTest, CreateDynamicBodyForEntity)
{
    flecs::world world;
    auto entity = world.entity("DynamicEntity");
    
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Dynamic;
    desc.mass = 1.0f;
    desc.position = glm::vec3(0.0f, 10.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(0.5f);
    
    PhysicsBodyHandle handle;
    bool created = context->createBodyForEntity(entity, desc, handle);
    EXPECT_TRUE(created);
    EXPECT_NE(handle, InvalidBodyHandle);
    
    context->destroyBodyForEntity(entity);
}

TEST_F(PhysicsContextTest, CreateBodyWithCustomTransform)
{
    flecs::world world;
    auto entity = world.entity("CustomTransformEntity");
    
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Static;
    desc.mass = 0.0f;
    desc.position = glm::vec3(5.0f, 10.0f, 15.0f);
    desc.rotation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(1.0f);
    
    PhysicsBodyHandle handle;
    bool created = context->createBodyForEntity(entity, desc, handle);
    EXPECT_TRUE(created);
    EXPECT_NE(handle, InvalidBodyHandle);
    
    glm::vec3 pos;
    glm::quat rot;
    bool gotTransform = context->getBodyTransform(entity, pos, rot);
    EXPECT_TRUE(gotTransform);
    EXPECT_NEAR(pos.x, 5.0f, 0.1f);
    EXPECT_NEAR(pos.y, 10.0f, 0.1f);
    EXPECT_NEAR(pos.z, 15.0f, 0.1f);
    
    context->destroyBodyForEntity(entity);
}

// ============================================================================
// Transform Update Tests
// ============================================================================

TEST_F(PhysicsContextTest, UpdateBodyTransform)
{
    flecs::world world;
    auto entity = world.entity("UpdateTransformEntity");
    
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Kinematic;
    desc.mass = 0.0f;
    desc.position = glm::vec3(0.0f, 0.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(1.0f);
    
    PhysicsBodyHandle handle;
    context->createBodyForEntity(entity, desc, handle);
    
    glm::vec3 newPos(10.0f, 20.0f, 30.0f);
    glm::quat newRot = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    bool updated = context->updateBodyTransform(entity, newPos, newRot);
    EXPECT_TRUE(updated);
    
    glm::vec3 pos;
    glm::quat rot;
    bool gotTransform = context->getBodyTransform(entity, pos, rot);
    EXPECT_TRUE(gotTransform);
    EXPECT_NEAR(pos.x, 10.0f, 0.1f);
    EXPECT_NEAR(pos.y, 20.0f, 0.1f);
    EXPECT_NEAR(pos.z, 30.0f, 0.1f);
    
    context->destroyBodyForEntity(entity);
}

// ============================================================================
// Debug Draw Tests
// ============================================================================

TEST_F(PhysicsContextTest, EnableDebugDraw)
{
    context->setDebugDrawEnabled(true);
    EXPECT_TRUE(context->isDebugDrawEnabled());
    
    context->setDebugDrawEnabled(false);
    EXPECT_FALSE(context->isDebugDrawEnabled());
}

TEST_F(PhysicsContextTest, StepWithDebugDrawEnabled)
{
    context->setDebugDrawEnabled(true);
    EXPECT_NO_THROW(context->step(1.0f / 60.0f));
}

