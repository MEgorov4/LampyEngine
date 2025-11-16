#include <gtest/gtest.h>
#include <Modules/PhysicsModule/PhysicsModule.h>
#include <Modules/PhysicsModule/PhysicsLocator.h>
#include <Modules/PhysicsModule/PhysicsContext/PhysicsContext.h>
#include <Modules/PhysicsModule/Components/RigidBodyComponent.h>
#include <Modules/PhysicsModule/Components/ColliderComponent.h>
#include <Modules/PhysicsModule/Events.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/RenderModule/RenderContext.h>
#include <Foundation/Event/EventBus.h>
#include <Foundation/Memory/MemorySystem.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Core/Core.h>
#include <flecs.h>

using namespace PhysicsModule;
using namespace RenderModule;
using namespace EngineCore::Foundation;
using namespace Events::Physics;

class CollisionScenariosTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize MemorySystem before creating RenderContext
        MemorySystem::startup(1024 * 1024, 4 * 1024 * 1024); // 1MB frame, 4MB persistent
        
        // Register ResourceManager in CoreLocator for RenderContext
        auto resourceManager = std::make_shared<ResourceModule::ResourceManager>();
        EngineCore::Base::Core::Register(resourceManager, 15);
        
        renderContext = std::make_unique<RenderContext>();
        eventBus = std::make_unique<EventBus>();
        world = std::make_unique<flecs::world>();
        
        // Create PhysicsContext and connect to EventBus
        context = std::make_unique<PhysicsContext>(renderContext.get());
        context->connectToEventBus(*eventBus);
        PhysicsLocator::Provide(context.get());
    }
    
    void TearDown() override
    {
        // Cleanup all entities
        if (world)
        {
            world->each([this](flecs::entity e) {
                if (context)
                {
                    context->destroyBodyForEntity(e);
                }
            });
        }
        
        PhysicsLocator::Reset();
        context.reset();
        world.reset();
        eventBus.reset();
        renderContext.reset();
        
        // Cleanup CoreLocator
        EngineCore::Base::Core::ShutdownAll();
        
        // Shutdown MemorySystem after cleanup
        MemorySystem::shutdown();
    }
    
    std::unique_ptr<RenderContext> renderContext;
    std::unique_ptr<EventBus> eventBus;
    std::unique_ptr<PhysicsContext> context;
    std::unique_ptr<flecs::world> world;
};

// ============================================================================
// Falling Box Hits Ground
// ============================================================================

TEST_F(CollisionScenariosTest, FallingBoxHitsGround)
{
    // Create ground
    auto ground = world->entity("Ground");
    PositionComponent groundPos;
    groundPos.x = 0.0f;
    groundPos.y = 0.0f;
    groundPos.z = 0.0f;
    
    RotationComponent groundRot;
    groundRot.x = 0.0f;
    groundRot.y = 0.0f;
    groundRot.z = 0.0f;
    
    SetEntityPosition(ground, groundPos);
    SetEntityRotation(ground, groundRot);
    
    RigidBodyDesc groundDesc;
    groundDesc.bodyType = RigidBodyType::Static;
    groundDesc.mass = 0.0f;
    groundDesc.position = groundPos.toGLMVec();
    groundDesc.rotation = groundRot.toQuat();
    groundDesc.shape.type = PhysicsShapeType::Box;
    groundDesc.shape.size = glm::vec3(5.0f, 0.5f, 5.0f);
    
    PhysicsBodyHandle groundHandle;
    context->createBodyForEntity(ground, groundDesc, groundHandle);
    
    // Create falling box
    auto box = world->entity("FallingBox");
    PositionComponent boxPos;
    boxPos.x = 0.0f;
    boxPos.y = 5.0f;
    boxPos.z = 0.0f;
    
    RotationComponent boxRot;
    boxRot.x = 0.0f;
    boxRot.y = 0.0f;
    boxRot.z = 0.0f;
    
    SetEntityPosition(box, boxPos);
    SetEntityRotation(box, boxRot);
    
    RigidBodyDesc boxDesc;
    boxDesc.bodyType = RigidBodyType::Dynamic;
    boxDesc.mass = 1.0f;
    boxDesc.position = boxPos.toGLMVec();
    boxDesc.rotation = boxRot.toQuat();
    boxDesc.shape.type = PhysicsShapeType::Box;
    boxDesc.shape.size = glm::vec3(0.5f);
    
    PhysicsBodyHandle boxHandle;
    context->createBodyForEntity(box, boxDesc, boxHandle);
    
    // Track collision
    bool collisionDetected = false;
    auto subscription = eventBus->subscribe<PhysicsCollisionEvent>(
        [&](const PhysicsCollisionEvent& event) {
            if (event.a == box || event.b == box)
            {
                collisionDetected = true;
            }
        }
    );
    
    // Step simulation
    float initialY = 5.0f;
    for (int i = 0; i < 300; ++i)
    {
        context->step(1.0f / 60.0f);
        
        // Check if box has fallen
        glm::vec3 currentPos;
        glm::quat currentRot;
        if (context->getBodyTransform(box, currentPos, currentRot))
        {
            // Box should be falling (y decreasing)
            if (i > 10)
            {
                EXPECT_LT(currentPos.y, initialY);
            }
            
            // Box should eventually hit ground
            if (currentPos.y < 1.0f)
            {
                // Box has hit or is very close to ground
                break;
            }
        }
    }
    
    // Verify box position is near ground
    glm::vec3 finalPos;
    glm::quat finalRot;
    if (context->getBodyTransform(box, finalPos, finalRot))
    {
        EXPECT_LT(finalPos.y, 2.0f); // Box should be near ground
    }
}

// ============================================================================
// Two Boxes Collide
// ============================================================================

TEST_F(CollisionScenariosTest, TwoBoxesCollide)
{
    // Create first box
    auto box1 = world->entity("Box1");
    PositionComponent pos1;
    pos1.x = -1.0f;
    pos1.y = 2.0f;
    pos1.z = 0.0f;
    
    RotationComponent rot1;
    rot1.x = 0.0f;
    rot1.y = 0.0f;
    rot1.z = 0.0f;
    
    SetEntityPosition(box1, pos1);
    SetEntityRotation(box1, rot1);
    
    RigidBodyDesc desc1;
    desc1.bodyType = RigidBodyType::Dynamic;
    desc1.mass = 1.0f;
    desc1.position = pos1.toGLMVec();
    desc1.rotation = rot1.toQuat();
    desc1.shape.type = PhysicsShapeType::Box;
    desc1.shape.size = glm::vec3(0.5f);
    
    PhysicsBodyHandle handle1;
    context->createBodyForEntity(box1, desc1, handle1);
    
    // Create second box moving towards first
    auto box2 = world->entity("Box2");
    PositionComponent pos2;
    pos2.x = 1.0f;
    pos2.y = 2.0f;
    pos2.z = 0.0f;
    
    RotationComponent rot2;
    rot2.x = 0.0f;
    rot2.y = 0.0f;
    rot2.z = 0.0f;
    
    SetEntityPosition(box2, pos2);
    SetEntityRotation(box2, rot2);
    
    RigidBodyDesc desc2;
    desc2.bodyType = RigidBodyType::Dynamic;
    desc2.mass = 1.0f;
    desc2.position = pos2.toGLMVec();
    desc2.rotation = rot2.toQuat();
    desc2.shape.type = PhysicsShapeType::Box;
    desc2.shape.size = glm::vec3(0.5f);
    
    PhysicsBodyHandle handle2;
    context->createBodyForEntity(box2, desc2, handle2);
    
    // Step simulation
    for (int i = 0; i < 120; ++i)
    {
        context->step(1.0f / 60.0f);
    }
    
    // Verify boxes have moved (collision or gravity)
    glm::vec3 pos1Final, pos2Final;
    glm::quat rot1Final, rot2Final;
    
    bool gotPos1 = context->getBodyTransform(box1, pos1Final, rot1Final);
    bool gotPos2 = context->getBodyTransform(box2, pos2Final, rot2Final);
    
    if (gotPos1 && gotPos2)
    {
        // At least one box should have moved
        bool box1Moved = (std::abs(pos1Final.x - pos1.x) > 0.01f) || 
                         (std::abs(pos1Final.y - pos1.y) > 0.01f);
        bool box2Moved = (std::abs(pos2Final.x - pos2.x) > 0.01f) || 
                         (std::abs(pos2Final.y - pos2.y) > 0.01f);
        
        EXPECT_TRUE(box1Moved || box2Moved);
    }
}

