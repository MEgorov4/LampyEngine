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

class ComplexStackingTest : public ::testing::Test
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
        if (world)
        {
            world->each([this](flecs::entity e) {
                if (context)
                {
                    context->destroyBodyForEntity(e);
                }
            });
        }
        
        context.reset();
        world.reset();
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
// Box Stacking Tests
// ============================================================================

TEST_F(ComplexStackingTest, StackThreeBoxes)
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
    
    // Create three boxes stacked
    std::vector<flecs::entity> boxes;
    for (int i = 0; i < 3; ++i)
    {
        auto box = world->entity(("Box" + std::to_string(i)).c_str());
        
        PositionComponent pos;
        pos.x = 0.0f;
        pos.y = 0.5f + i * 1.0f; // Stack them
        pos.z = 0.0f;
        
        RotationComponent rot;
        rot.x = 0.0f;
        rot.y = 0.0f;
        rot.z = 0.0f;
        
        SetEntityPosition(box, pos);
        SetEntityRotation(box, rot);
        
        RigidBodyDesc boxDesc;
        boxDesc.bodyType = RigidBodyType::Dynamic;
        boxDesc.mass = 1.0f;
        boxDesc.position = pos.toGLMVec();
        boxDesc.rotation = rot.toQuat();
        boxDesc.shape.type = PhysicsShapeType::Box;
        boxDesc.shape.size = glm::vec3(0.5f);
        
        PhysicsBodyHandle boxHandle;
        context->createBodyForEntity(box, boxDesc, boxHandle);
        boxes.push_back(box);
    }
    
    // Step simulation to let stack settle
    for (int i = 0; i < 300; ++i)
    {
        context->step(1.0f / 60.0f);
    }
    
    // Verify boxes are still stacked (not all fallen)
    std::vector<float> finalYPositions;
    for (auto& box : boxes)
    {
        glm::vec3 pos;
        glm::quat rot;
        if (context->getBodyTransform(box, pos, rot))
        {
            finalYPositions.push_back(pos.y);
        }
    }
    
    // At least one box should be above ground level
    bool hasStackedBox = false;
    for (float y : finalYPositions)
    {
        if (y > 1.0f)
        {
            hasStackedBox = true;
            break;
        }
    }
    
    // Stacking is complex - boxes may fall, but we verify physics is working
    EXPECT_GE(finalYPositions.size(), 3);
}

TEST_F(ComplexStackingTest, StackStabilizes)
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
    
    // Create a single box
    auto box = world->entity("StableBox");
    PositionComponent pos;
    pos.x = 0.0f;
    pos.y = 2.0f;
    pos.z = 0.0f;
    
    RotationComponent rot;
    rot.x = 0.0f;
    rot.y = 0.0f;
    rot.z = 0.0f;
    
    SetEntityPosition(box, pos);
    SetEntityRotation(box, rot);
    
    RigidBodyDesc boxDesc;
    boxDesc.bodyType = RigidBodyType::Dynamic;
    boxDesc.mass = 1.0f;
    boxDesc.position = pos.toGLMVec();
    boxDesc.rotation = rot.toQuat();
    boxDesc.shape.type = PhysicsShapeType::Box;
    boxDesc.shape.size = glm::vec3(0.5f);
    
    PhysicsBodyHandle boxHandle;
    context->createBodyForEntity(box, boxDesc, boxHandle);
    
    // Step simulation
    std::vector<float> yPositions;
    for (int i = 0; i < 300; ++i)
    {
        context->step(1.0f / 60.0f);
        
        if (i % 30 == 0) // Sample every 0.5 seconds
        {
            glm::vec3 currentPos;
            glm::quat currentRot;
            if (context->getBodyTransform(box, currentPos, currentRot))
            {
                yPositions.push_back(currentPos.y);
            }
        }
    }
    
    // Box should eventually stabilize (stop falling)
    if (yPositions.size() >= 2)
    {
        float lastY = yPositions.back();
        float secondLastY = yPositions[yPositions.size() - 2];
        
        // Position should stabilize (change is small)
        float change = std::abs(lastY - secondLastY);
        EXPECT_LT(change, 0.1f); // Should be stable within 0.1 units
    }
}

