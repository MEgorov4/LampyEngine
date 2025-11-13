#include <gtest/gtest.h>
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

class TriggerVolumesTest : public ::testing::Test
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
        context = std::make_unique<PhysicsContext>(renderContext.get());
        context->connectToEventBus(*eventBus);
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
// Trigger Volume Tests
// ============================================================================

TEST_F(TriggerVolumesTest, TriggerVolumeSetup)
{
    // Create a trigger volume (static body with isTrigger flag)
    auto trigger = world->entity("TriggerVolume");
    
    PositionComponent pos;
    pos.x = 0.0f;
    pos.y = 0.0f;
    pos.z = 0.0f;
    
    RotationComponent rot;
    rot.x = 0.0f;
    rot.y = 0.0f;
    rot.z = 0.0f;
    
    trigger.set<PositionComponent>(pos);
    trigger.set<RotationComponent>(rot);
    
    ColliderComponent collider;
    collider.shapeDesc.type = PhysicsShapeType::Box;
    collider.shapeDesc.size = glm::vec3(2.0f);
    collider.isTrigger = true;
    
    trigger.set<ColliderComponent>(collider);
    
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Static;
    desc.mass = 0.0f;
    desc.position = pos.toGLMVec();
    desc.rotation = rot.toQuat();
    desc.shape = collider.shapeDesc;
    
    PhysicsBodyHandle triggerHandle;
    context->createBodyForEntity(trigger, desc, triggerHandle);
    
    // Verify trigger was created
    glm::vec3 triggerPos;
    glm::quat triggerRot;
    bool gotTransform = context->getBodyTransform(trigger, triggerPos, triggerRot);
    EXPECT_TRUE(gotTransform);
    
    context->destroyBodyForEntity(trigger);
}

TEST_F(TriggerVolumesTest, ObjectEntersTriggerVolume)
{
    // Create trigger volume
    auto trigger = world->entity("Trigger");
    
    PositionComponent triggerPos;
    triggerPos.x = 0.0f;
    triggerPos.y = 0.0f;
    triggerPos.z = 0.0f;
    
    RotationComponent triggerRot;
    triggerRot.x = 0.0f;
    triggerRot.y = 0.0f;
    triggerRot.z = 0.0f;
    
    trigger.set<PositionComponent>(triggerPos);
    trigger.set<RotationComponent>(triggerRot);
    
    ColliderComponent triggerCollider;
    triggerCollider.shapeDesc.type = PhysicsShapeType::Box;
    triggerCollider.shapeDesc.size = glm::vec3(2.0f);
    triggerCollider.isTrigger = true;
    
    RigidBodyDesc triggerDesc;
    triggerDesc.bodyType = RigidBodyType::Static;
    triggerDesc.mass = 0.0f;
    triggerDesc.position = triggerPos.toGLMVec();
    triggerDesc.rotation = triggerRot.toQuat();
    triggerDesc.shape = triggerCollider.shapeDesc;
    
    PhysicsBodyHandle triggerHandle;
    context->createBodyForEntity(trigger, triggerDesc, triggerHandle);
    
    // Create object that will enter trigger
    auto object = world->entity("Object");
    
    PositionComponent objPos;
    objPos.x = 0.0f;
    objPos.y = 5.0f;
    objPos.z = 0.0f;
    
    RotationComponent objRot;
    objRot.x = 0.0f;
    objRot.y = 0.0f;
    objRot.z = 0.0f;
    
    object.set<PositionComponent>(objPos);
    object.set<RotationComponent>(objRot);
    
    RigidBodyDesc objDesc;
    objDesc.bodyType = RigidBodyType::Dynamic;
    objDesc.mass = 1.0f;
    objDesc.position = objPos.toGLMVec();
    objDesc.rotation = objRot.toQuat();
    objDesc.shape.type = PhysicsShapeType::Box;
    objDesc.shape.size = glm::vec3(0.5f);
    
    PhysicsBodyHandle objHandle;
    context->createBodyForEntity(object, objDesc, objHandle);
    
    // Track trigger events
    bool triggerEntered = false;
    auto subscription = eventBus->subscribe<PhysicsTriggerEvent>(
        [&](const PhysicsTriggerEvent& event) {
            if (event.trigger == trigger && event.other == object)
            {
                triggerEntered = true;
            }
        }
    );
    
    // Step simulation - object should fall into trigger
    for (int i = 0; i < 300; ++i)
    {
        context->step(1.0f / 60.0f);
        
        // Check if object has entered trigger volume
        glm::vec3 currentPos;
        glm::quat currentRot;
        if (context->getBodyTransform(object, currentPos, currentRot))
        {
            // Object is in trigger volume if y is near 0
            if (std::abs(currentPos.y) < 1.5f && std::abs(currentPos.x) < 1.5f && std::abs(currentPos.z) < 1.5f)
            {
                // Object is in trigger volume
                break;
            }
        }
    }
    
    // Verify object entered trigger area
    glm::vec3 finalPos;
    glm::quat finalRot;
    if (context->getBodyTransform(object, finalPos, finalRot))
    {
        // Object should be near trigger center
        float distance = glm::length(finalPos - triggerPos.toGLMVec());
        EXPECT_LT(distance, 2.0f);
    }
    
    context->destroyBodyForEntity(trigger);
    context->destroyBodyForEntity(object);
}

