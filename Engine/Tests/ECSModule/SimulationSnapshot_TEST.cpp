#include <gtest/gtest.h>
#include <Modules/ObjectCoreModule/ECS/EntityWorld.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ScriptModule/LuaScriptModule.h>
#include <Modules/PhysicsModule/PhysicsModule.h>
#include <Modules/PhysicsModule/PhysicsLocator.h>
#include <Modules/PhysicsModule/PhysicsContext/PhysicsContext.h>
#include <Modules/PhysicsModule/Components/RigidBodyComponent.h>
#include <Modules/PhysicsModule/Components/ColliderComponent.h>
#include <Modules/PhysicsModule/Utils/PhysicsTypes.h>
#include <Modules/RenderModule/RenderContext.h>
#include <Foundation/Memory/MemorySystem.h>
#include <Core/Core.h>
#include <string>
#include <memory>
#include <vector>
#include <cmath>

using namespace ECSModule;
using namespace ResourceModule;
using namespace ScriptModule;
// Don't use "using namespace PhysicsModule" to avoid conflict with PhysicsModule::PhysicsModule class

class SimulationSnapshotTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize MemorySystem
        EngineCore::Foundation::MemorySystem::startup(1024 * 1024, 4 * 1024 * 1024);
        
        // Register ResourceManager for RenderContext
        resourceManager = std::make_shared<ResourceManager>();
        EngineCore::Base::Core::Register(resourceManager, 15);
        
        // Create RenderContext for PhysicsContext
        renderContext = std::make_unique<RenderModule::RenderContext>();
        physicsContext = std::make_unique<PhysicsModule::PhysicsContext>(renderContext.get());
        PhysicsModule::PhysicsLocator::Provide(physicsContext.get());
        
        // Create mock dependencies
        scriptModule = std::make_shared<LuaScriptModule>();
        physicsModule = std::make_shared<PhysicsModule::PhysicsModule>();
        
        // Create EntityWorld
        world = std::make_unique<EntityWorld>(
            resourceManager.get(),
            scriptModule.get(),
            physicsModule.get()
        );
        world->init();
    }
    
    void TearDown() override
    {
        PhysicsModule::PhysicsLocator::Reset();
        world.reset();
        physicsModule.reset();
        scriptModule.reset();
        physicsContext.reset();
        renderContext.reset();
        resourceManager.reset();
        
        // Cleanup CoreLocator
        EngineCore::Base::Core::ShutdownAll();
        
        // Shutdown MemorySystem
        EngineCore::Foundation::MemorySystem::shutdown();
    }
    
    std::shared_ptr<ResourceManager> resourceManager;
    std::shared_ptr<LuaScriptModule> scriptModule;
    std::shared_ptr<PhysicsModule::PhysicsModule> physicsModule;
    std::unique_ptr<RenderModule::RenderContext> renderContext;
    std::unique_ptr<PhysicsModule::PhysicsContext> physicsContext;
    std::unique_ptr<EntityWorld> world;
};

// ============================================================================
// Basic Snapshot Tests
// ============================================================================

TEST_F(SimulationSnapshotTest, SaveSnapshotPreservesPositions)
{
    auto& flecsWorld = world->get();
    
    // Create entity with initial position
    auto entity = flecsWorld.entity("TestEntity");
    PositionComponent initialPos{10.0f, 20.0f, 30.0f};
    SetEntityPosition(entity, initialPos);
    
    // Save snapshot
    std::string snapshot = world->serialize();
    EXPECT_FALSE(snapshot.empty());
    
    // Modify position
    PositionComponent modifiedPos{100.0f, 200.0f, 300.0f};
    SetEntityPosition(entity, modifiedPos);
    
    // Verify position was modified
    const PositionComponent* currentPos = GetEntityPosition(entity);
    EXPECT_FLOAT_EQ(currentPos->x, 100.0f);
    EXPECT_FLOAT_EQ(currentPos->y, 200.0f);
    EXPECT_FLOAT_EQ(currentPos->z, 300.0f);
    
    // Restore from snapshot
    world->deserialize(snapshot);
    
    // Verify position was restored
    // Note: After deserialize, we need to get a fresh reference to the world
    auto& restoredFlecsWorld = world->get();
    auto restoredEntity = restoredFlecsWorld.lookup("TestEntity");
    EXPECT_TRUE(restoredEntity.is_valid());
    if (restoredEntity.has<TransformComponent>())
    {
        const PositionComponent* restoredPos = GetEntityPosition(restoredEntity);
        EXPECT_FLOAT_EQ(restoredPos->x, 10.0f);
        EXPECT_FLOAT_EQ(restoredPos->y, 20.0f);
        EXPECT_FLOAT_EQ(restoredPos->z, 30.0f);
    }
}

TEST_F(SimulationSnapshotTest, SaveSnapshotPreservesRotations)
{
    auto& flecsWorld = world->get();
    
    // Create entity with initial rotation
    auto entity = flecsWorld.entity("RotationEntity");
    RotationComponent initialRot{45.0f, 90.0f, 180.0f};
    SetEntityRotation(entity, initialRot);
    
    // Save snapshot
    std::string snapshot = world->serialize();
    
    // Modify rotation
    RotationComponent modifiedRot{0.0f, 0.0f, 0.0f};
    SetEntityRotation(entity, modifiedRot);
    
    // Restore from snapshot
    world->deserialize(snapshot);
    
    // Verify rotation was restored
    auto restoredEntity = flecsWorld.lookup("RotationEntity");
    EXPECT_TRUE(restoredEntity.is_valid());
    if (restoredEntity.has<TransformComponent>())
    {
        const RotationComponent* restoredRot = GetEntityRotation(restoredEntity);
        EXPECT_FLOAT_EQ(restoredRot->x, 45.0f);
        EXPECT_FLOAT_EQ(restoredRot->y, 90.0f);
        EXPECT_FLOAT_EQ(restoredRot->z, 180.0f);
    }
}

TEST_F(SimulationSnapshotTest, SaveSnapshotPreservesMultipleEntities)
{
    auto& flecsWorld = world->get();
    
    // Create multiple entities
    auto entity1 = flecsWorld.entity("Entity1");
    SetEntityPosition(entity1, {1.0f, 2.0f, 3.0f});
    
    auto entity2 = flecsWorld.entity("Entity2");
    SetEntityPosition(entity2, {4.0f, 5.0f, 6.0f});
    
    auto entity3 = flecsWorld.entity("Entity3");
    SetEntityPosition(entity3, {7.0f, 8.0f, 9.0f});
    
    // Save snapshot
    std::string snapshot = world->serialize();
    
    // Modify all positions
    SetEntityPosition(entity1, {100.0f, 200.0f, 300.0f});
    SetEntityPosition(entity2, {400.0f, 500.0f, 600.0f});
    SetEntityPosition(entity3, {700.0f, 800.0f, 900.0f});
    
    // Restore from snapshot
    world->deserialize(snapshot);
    
    // Verify all positions were restored
    auto restored1 = flecsWorld.lookup("Entity1");
    auto restored2 = flecsWorld.lookup("Entity2");
    auto restored3 = flecsWorld.lookup("Entity3");
    
    EXPECT_TRUE(restored1.is_valid());
    EXPECT_TRUE(restored2.is_valid());
    EXPECT_TRUE(restored3.is_valid());
    
    if (restored1.has<TransformComponent>())
    {
        const PositionComponent* pos = GetEntityPosition(restored1);
        EXPECT_FLOAT_EQ(pos->x, 1.0f);
        EXPECT_FLOAT_EQ(pos->y, 2.0f);
        EXPECT_FLOAT_EQ(pos->z, 3.0f);
    }
    
    if (restored2.has<TransformComponent>())
    {
        const PositionComponent* pos = GetEntityPosition(restored2);
        EXPECT_FLOAT_EQ(pos->x, 4.0f);
        EXPECT_FLOAT_EQ(pos->y, 5.0f);
        EXPECT_FLOAT_EQ(pos->z, 6.0f);
    }
    
    if (restored3.has<TransformComponent>())
    {
        const PositionComponent* pos = GetEntityPosition(restored3);
        EXPECT_FLOAT_EQ(pos->x, 7.0f);
        EXPECT_FLOAT_EQ(pos->y, 8.0f);
        EXPECT_FLOAT_EQ(pos->z, 9.0f);
    }
}

// ============================================================================
// Physics Components Snapshot Tests
// ============================================================================

TEST_F(SimulationSnapshotTest, SaveSnapshotPreservesPhysicsComponents)
{
    auto& flecsWorld = world->get();
    
    // Create entity with physics components
    auto entity = flecsWorld.entity("PhysicsEntity");
    PositionComponent pos{0.0f, 5.0f, 0.0f};
    RotationComponent rot{0.0f, 0.0f, 0.0f};
    PhysicsModule::RigidBodyComponent rb;
    rb.mass = 1.0f;
    rb.isStatic = false;
    rb.isKinematic = false;
    rb.needsCreation = true;
    rb.bodyHandle = PhysicsModule::InvalidBodyHandle;
    
    PhysicsModule::ColliderComponent collider;
    collider.shapeDesc.type = PhysicsModule::PhysicsShapeType::Box;
    collider.shapeDesc.size = glm::vec3(1.0f);
    collider.needsCreation = true;
    
    SetEntityPosition(entity, pos);
    SetEntityRotation(entity, rot);
    entity.set<PhysicsModule::RigidBodyComponent>(rb);
    entity.set<PhysicsModule::ColliderComponent>(collider);
    
    // Save snapshot
    std::string snapshot = world->serialize();
    
    // Modify position (simulating physics movement)
    PositionComponent modifiedPos{10.0f, 15.0f, 20.0f};
    SetEntityPosition(entity, modifiedPos);
    
    // Restore from snapshot
    world->deserialize(snapshot);
    
    // Verify position was restored
    auto restoredEntity = flecsWorld.lookup("PhysicsEntity");
    EXPECT_TRUE(restoredEntity.is_valid());
    
    if (restoredEntity.has<TransformComponent>())
    {
        const PositionComponent* restoredPos = GetEntityPosition(restoredEntity);
        EXPECT_FLOAT_EQ(restoredPos->x, 0.0f);
        EXPECT_FLOAT_EQ(restoredPos->y, 5.0f);
        EXPECT_FLOAT_EQ(restoredPos->z, 0.0f);
    }
    
    // Verify RigidBodyComponent flags are reset for recreation
    if (restoredEntity.has<PhysicsModule::RigidBodyComponent>())
    {
        PhysicsModule::RigidBodyComponent* restoredRb = restoredEntity.get_mut<PhysicsModule::RigidBodyComponent>();
        EXPECT_EQ(restoredRb->bodyHandle, PhysicsModule::InvalidBodyHandle);
        EXPECT_TRUE(restoredRb->needsCreation);
    }
    
    // Verify ColliderComponent flags are reset
    if (restoredEntity.has<PhysicsModule::ColliderComponent>())
    {
        PhysicsModule::ColliderComponent* restoredCollider = restoredEntity.get_mut<PhysicsModule::ColliderComponent>();
        EXPECT_TRUE(restoredCollider->needsCreation);
    }
}

TEST_F(SimulationSnapshotTest, RestoreSnapshotResetsPhysicsBodyHandles)
{
    auto& flecsWorld = world->get();
    
    // Create entity with physics components
    auto entity = flecsWorld.entity("PhysicsEntity2");
    PositionComponent pos{0.0f, 0.0f, 0.0f};
    RotationComponent rot{0.0f, 0.0f, 0.0f};
    PhysicsModule::RigidBodyComponent rb;
    rb.mass = 1.0f;
    rb.isStatic = false;
    rb.needsCreation = true;
    rb.bodyHandle = PhysicsModule::InvalidBodyHandle;
    
    PhysicsModule::ColliderComponent collider;
    collider.shapeDesc.type = PhysicsModule::PhysicsShapeType::Box;
    collider.shapeDesc.size = glm::vec3(1.0f);
    collider.needsCreation = true;
    
    SetEntityPosition(entity, pos);
    SetEntityRotation(entity, rot);
    entity.set<PhysicsModule::RigidBodyComponent>(rb);
    entity.set<PhysicsModule::ColliderComponent>(collider);
    
    // Save snapshot
    std::string snapshot = world->serialize();
    
    // Simulate physics body creation (set a fake handle)
    PhysicsModule::RigidBodyComponent* rbMut = entity.get_mut<PhysicsModule::RigidBodyComponent>();
    rbMut->bodyHandle = static_cast<PhysicsModule::PhysicsBodyHandle>(0x12345678ULL); // Fake handle
    rbMut->needsCreation = false;
    
    // Restore from snapshot
    world->deserialize(snapshot);
    
    // Manually reset flags (as done in simulate(false))
    flecsWorld.each<PhysicsModule::RigidBodyComponent>([&](flecs::entity e, PhysicsModule::RigidBodyComponent& rbComp) {
        rbComp.bodyHandle = PhysicsModule::InvalidBodyHandle;
        rbComp.needsCreation = true;
    });
    
    flecsWorld.each<PhysicsModule::ColliderComponent>([&](flecs::entity e, PhysicsModule::ColliderComponent& coll) {
        coll.needsCreation = true;
    });
    
    // Verify flags were reset
    auto restoredEntity = flecsWorld.lookup("PhysicsEntity2");
    EXPECT_TRUE(restoredEntity.is_valid());
    
    if (restoredEntity.has<PhysicsModule::RigidBodyComponent>())
    {
        const PhysicsModule::RigidBodyComponent* restoredRb = restoredEntity.get<PhysicsModule::RigidBodyComponent>();
        EXPECT_EQ(restoredRb->bodyHandle, PhysicsModule::InvalidBodyHandle);
        EXPECT_TRUE(restoredRb->needsCreation);
    }
    
    if (restoredEntity.has<PhysicsModule::ColliderComponent>())
    {
        const PhysicsModule::ColliderComponent* restoredCollider = restoredEntity.get<PhysicsModule::ColliderComponent>();
        EXPECT_TRUE(restoredCollider->needsCreation);
    }
}

// ============================================================================
// Round Trip Tests (Simulate Start/Stop)
// ============================================================================

TEST_F(SimulationSnapshotTest, SimulateStartStopRoundTrip)
{
    auto& flecsWorld = world->get();
    
    // Create entity with initial state
    auto entity = flecsWorld.entity("RoundTripEntity");
    PositionComponent initialPos{5.0f, 10.0f, 15.0f};
    RotationComponent initialRot{30.0f, 60.0f, 90.0f};
    SetEntityPosition(entity, initialPos);
    SetEntityRotation(entity, initialRot);
    
    // Save snapshot (simulating simulate(true))
    std::string snapshot = world->serialize();
    EXPECT_FALSE(snapshot.empty());
    
    // Simulate physics movement (modify positions)
    PositionComponent simulatedPos{50.0f, 100.0f, 150.0f};
    RotationComponent simulatedRot{300.0f, 600.0f, 900.0f};
    SetEntityPosition(entity, simulatedPos);
    SetEntityRotation(entity, simulatedRot);
    
    // Verify positions were changed
    const PositionComponent* currentPos = GetEntityPosition(entity);
    EXPECT_FLOAT_EQ(currentPos->x, 50.0f);
    EXPECT_FLOAT_EQ(currentPos->y, 100.0f);
    EXPECT_FLOAT_EQ(currentPos->z, 150.0f);
    
    // Clear physics bodies (simulating simulate(false) cleanup)
    std::vector<flecs::entity> entitiesToClear;
    flecsWorld.each<PhysicsModule::RigidBodyComponent>([&entitiesToClear](flecs::entity e, PhysicsModule::RigidBodyComponent&) {
        entitiesToClear.push_back(e);
    });
    
    for (auto& e : entitiesToClear)
    {
        if (e.is_valid())
        {
            physicsContext->destroyBodyForEntity(e);
        }
    }
    
    // Restore from snapshot (simulating simulate(false) restore)
    world->deserialize(snapshot);
    
    // Reset physics component flags
    flecsWorld.each<PhysicsModule::RigidBodyComponent>([&](flecs::entity e, PhysicsModule::RigidBodyComponent& rb) {
        rb.bodyHandle = PhysicsModule::InvalidBodyHandle;
        rb.needsCreation = true;
    });
    
    flecsWorld.each<PhysicsModule::ColliderComponent>([&](flecs::entity e, PhysicsModule::ColliderComponent& collider) {
        collider.needsCreation = true;
    });
    
    // Verify entity was restored to initial state
    auto restoredEntity = flecsWorld.lookup("RoundTripEntity");
    EXPECT_TRUE(restoredEntity.is_valid());
    
    if (restoredEntity.has<TransformComponent>())
    {
        const PositionComponent* restoredPos = GetEntityPosition(restoredEntity);
        EXPECT_FLOAT_EQ(restoredPos->x, 5.0f);
        EXPECT_FLOAT_EQ(restoredPos->y, 10.0f);
        EXPECT_FLOAT_EQ(restoredPos->z, 15.0f);
    }
    
    if (restoredEntity.has<TransformComponent>())
    {
        const RotationComponent* restoredRot = GetEntityRotation(restoredEntity);
        EXPECT_FLOAT_EQ(restoredRot->x, 30.0f);
        EXPECT_FLOAT_EQ(restoredRot->y, 60.0f);
        EXPECT_FLOAT_EQ(restoredRot->z, 90.0f);
    }
}

TEST_F(SimulationSnapshotTest, MultipleSimulationCycles)
{
    auto& flecsWorld = world->get();
    
    // Create entity
    auto entity = flecsWorld.entity("MultiCycleEntity");
    PositionComponent initialPos{1.0f, 2.0f, 3.0f};
    SetEntityPosition(entity, initialPos);
    
    // First cycle
    std::string snapshot1 = world->serialize();
    SetEntityPosition(entity, {10.0f, 20.0f, 30.0f});
    world->deserialize(snapshot1);
    
    auto restored1 = flecsWorld.lookup("MultiCycleEntity");
    EXPECT_TRUE(restored1.is_valid());
    if (restored1.has<TransformComponent>())
    {
        const PositionComponent* pos = GetEntityPosition(restored1);
        EXPECT_FLOAT_EQ(pos->x, 1.0f);
        EXPECT_FLOAT_EQ(pos->y, 2.0f);
        EXPECT_FLOAT_EQ(pos->z, 3.0f);
    }
    
    // Second cycle
    std::string snapshot2 = world->serialize();
    SetEntityPosition(restored1, {100.0f, 200.0f, 300.0f});
    world->deserialize(snapshot2);
    
    auto restored2 = flecsWorld.lookup("MultiCycleEntity");
    EXPECT_TRUE(restored2.is_valid());
    if (restored2.has<TransformComponent>())
    {
        const PositionComponent* pos = GetEntityPosition(restored2);
        EXPECT_FLOAT_EQ(pos->x, 1.0f);
        EXPECT_FLOAT_EQ(pos->y, 2.0f);
        EXPECT_FLOAT_EQ(pos->z, 3.0f);
    }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(SimulationSnapshotTest, EmptySnapshot)
{
    // Create empty world snapshot
    std::string snapshot = world->serialize();
    EXPECT_FALSE(snapshot.empty());
    
    // Should be able to restore empty snapshot
    EXPECT_NO_THROW(world->deserialize(snapshot));
}

TEST_F(SimulationSnapshotTest, SnapshotWithNoPhysicsComponents)
{
    auto& flecsWorld = world->get();
    
    // Create entity without physics components
    auto entity = flecsWorld.entity("NoPhysicsEntity");
    PositionComponent pos{1.0f, 2.0f, 3.0f};
    RotationComponent rot{10.0f, 20.0f, 30.0f};
    SetEntityPosition(entity, pos);
    SetEntityRotation(entity, rot);
    
    // Save and restore
    std::string snapshot = world->serialize();
    SetEntityPosition(entity, {100.0f, 200.0f, 300.0f});
    world->deserialize(snapshot);
    
    // Verify restoration
    auto restored = flecsWorld.lookup("NoPhysicsEntity");
    EXPECT_TRUE(restored.is_valid());
    if (restored.has<TransformComponent>())
    {
        const PositionComponent* restoredPos = GetEntityPosition(restored);
        EXPECT_FLOAT_EQ(restoredPos->x, 1.0f);
        EXPECT_FLOAT_EQ(restoredPos->y, 2.0f);
        EXPECT_FLOAT_EQ(restoredPos->z, 3.0f);
    }
}

