#include <gtest/gtest.h>
#include <Modules/PhysicsModule/PhysicsContext/PhysicsContext.h>
#include <Modules/PhysicsModule/Components/CharacterControllerComponent.h>
#include <Modules/PhysicsModule/Components/RigidBodyComponent.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/RenderModule/RenderContext.h>
#include <Foundation/Memory/MemorySystem.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Core/Core.h>
#include <flecs.h>

using namespace PhysicsModule;
using namespace RenderModule;

class CharacterControllerMovementTest : public ::testing::Test
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
// Character Controller Tests
// ============================================================================

TEST_F(CharacterControllerMovementTest, CharacterControllerComponentCreation)
{
    auto character = world->entity("Character");
    
    CharacterControllerComponent controller;
    controller.radius = 0.5f;
    controller.height = 2.0f;
    controller.stepHeight = 0.3f;
    controller.velocity = glm::vec3(0.0f);
    controller.isGrounded = false;
    
    character.set<CharacterControllerComponent>(controller);
    
    EXPECT_TRUE(character.has<CharacterControllerComponent>());
    
    auto* cc = character.get<CharacterControllerComponent>();
    ASSERT_NE(cc, nullptr);
    EXPECT_FLOAT_EQ(cc->radius, 0.5f);
    EXPECT_FLOAT_EQ(cc->height, 2.0f);
    EXPECT_FLOAT_EQ(cc->stepHeight, 0.3f);
}

TEST_F(CharacterControllerMovementTest, CharacterControllerWithPhysicsBody)
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
    groundDesc.shape.size = glm::vec3(10.0f, 0.5f, 10.0f);
    
    PhysicsBodyHandle groundHandle;
    context->createBodyForEntity(ground, groundDesc, groundHandle);
    
    // Create character
    auto character = world->entity("Character");
    
    PositionComponent charPos;
    charPos.x = 0.0f;
    charPos.y = 3.0f;
    charPos.z = 0.0f;
    
    RotationComponent charRot;
    charRot.x = 0.0f;
    charRot.y = 0.0f;
    charRot.z = 0.0f;
    
    SetEntityPosition(character, charPos);
    SetEntityRotation(character, charRot);
    
    CharacterControllerComponent controller;
    controller.radius = 0.5f;
    controller.height = 2.0f;
    controller.stepHeight = 0.3f;
    controller.velocity = glm::vec3(0.0f);
    controller.isGrounded = false;
    
    character.set<CharacterControllerComponent>(controller);
    
    // Create physics body for character (capsule shape)
    RigidBodyDesc charDesc;
    charDesc.bodyType = RigidBodyType::Kinematic; // Character controllers are typically kinematic
    charDesc.mass = 0.0f;
    charDesc.position = charPos.toGLMVec();
    charDesc.rotation = charRot.toQuat();
    charDesc.shape.type = PhysicsShapeType::Capsule;
    charDesc.shape.radius = controller.radius;
    charDesc.shape.height = controller.height;
    
    PhysicsBodyHandle charHandle;
    context->createBodyForEntity(character, charDesc, charHandle);
    
    // Step simulation
    for (int i = 0; i < 120; ++i)
    {
        context->step(1.0f / 60.0f);
    }
    
    // Verify character position
    glm::vec3 finalPos;
    glm::quat finalRot;
    bool gotTransform = context->getBodyTransform(character, finalPos, finalRot);
    EXPECT_TRUE(gotTransform);
    
    // Character should be above ground (or on it if kinematic)
    EXPECT_GE(finalPos.y, 0.0f);
    
    context->destroyBodyForEntity(ground);
    context->destroyBodyForEntity(character);
}

TEST_F(CharacterControllerMovementTest, CharacterControllerVelocity)
{
    auto character = world->entity("Character");
    
    CharacterControllerComponent controller;
    controller.radius = 0.5f;
    controller.height = 2.0f;
    controller.velocity = glm::vec3(1.0f, 0.0f, 0.0f); // Moving in X direction
    
    character.set<CharacterControllerComponent>(controller);
    
    auto* cc = character.get<CharacterControllerComponent>();
    ASSERT_NE(cc, nullptr);
    EXPECT_FLOAT_EQ(cc->velocity.x, 1.0f);
    EXPECT_FLOAT_EQ(cc->velocity.y, 0.0f);
    EXPECT_FLOAT_EQ(cc->velocity.z, 0.0f);
}

