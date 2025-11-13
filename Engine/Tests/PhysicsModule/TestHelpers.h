#pragma once

#include <gtest/gtest.h>
#include <Modules/PhysicsModule/PhysicsContext/PhysicsContext.h>
#include <Modules/PhysicsModule/Utils/PhysicsTypes.h>
#include <Modules/RenderModule/RenderContext.h>
#include <Foundation/Event/EventBus.h>
#include <Foundation/Memory/MemorySystem.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Core/Core.h>
#include <flecs.h>
#include <memory>
#include <glm/glm.hpp>

namespace PhysicsModuleTest
{
    // Base class for PhysicsModule tests
    class PhysicsModuleTestBase : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Initialize MemorySystem before creating RenderContext
            // RenderContext requires MemorySystem to be initialized
            EngineCore::Foundation::MemorySystem::startup(1024 * 1024, 4 * 1024 * 1024); // 1MB frame, 4MB persistent
            
            // Register a mock ResourceManager in CoreLocator for RenderContext
            // RenderContext requires ResourceManager to be available
            auto resourceManager = std::make_shared<ResourceModule::ResourceManager>();
            EngineCore::Base::Core::Register(resourceManager, 15);
            
            renderContext = std::make_unique<RenderModule::RenderContext>();
            context = std::make_unique<PhysicsModule::PhysicsContext>(renderContext.get());
            eventBus = std::make_unique<EngineCore::Foundation::EventBus>();
            world = std::make_unique<flecs::world>();
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
            
            world.reset();
            context.reset();
            eventBus.reset();
            renderContext.reset();
            
            // Cleanup CoreLocator
            EngineCore::Base::Core::ShutdownAll();
            
            // Shutdown MemorySystem after cleanup
            EngineCore::Foundation::MemorySystem::shutdown();
        }
        
        std::unique_ptr<RenderModule::RenderContext> renderContext;
        std::unique_ptr<PhysicsModule::PhysicsContext> context;
        std::unique_ptr<EngineCore::Foundation::EventBus> eventBus;
        std::unique_ptr<flecs::world> world;
    };
    
    // Helper functions for test setup
    namespace Setup
    {
        // Initialize MemorySystem and register ResourceManager
        inline void InitializeTestEnvironment()
        {
            EngineCore::Foundation::MemorySystem::startup(1024 * 1024, 4 * 1024 * 1024);
            auto resourceManager = std::make_shared<ResourceModule::ResourceManager>();
            EngineCore::Base::Core::Register(resourceManager, 15);
        }
        
        // Cleanup test environment
        inline void CleanupTestEnvironment()
        {
            EngineCore::Base::Core::ShutdownAll();
            EngineCore::Foundation::MemorySystem::shutdown();
        }
    }
    
    // Helper functions
    namespace Helpers
    {
        // Create a static ground plane
        flecs::entity CreateGround(flecs::world& world, 
                                   PhysicsModule::PhysicsContext& ctx,
                                   const glm::vec3& position = glm::vec3(0.0f),
                                   const glm::vec3& size = glm::vec3(10.0f, 0.5f, 10.0f));
        
        // Create a dynamic box
        flecs::entity CreateDynamicBox(flecs::world& world,
                                       PhysicsModule::PhysicsContext& ctx,
                                       const glm::vec3& position,
                                       const glm::vec3& size = glm::vec3(0.5f),
                                       float mass = 1.0f);
        
        // Create a static box
        flecs::entity CreateStaticBox(flecs::world& world,
                                      PhysicsModule::PhysicsContext& ctx,
                                      const glm::vec3& position,
                                      const glm::vec3& size = glm::vec3(1.0f));
        
        // Create a sphere
        flecs::entity CreateSphere(flecs::world& world,
                                    PhysicsModule::PhysicsContext& ctx,
                                    const glm::vec3& position,
                                    float radius = 1.0f,
                                    PhysicsModule::RigidBodyType bodyType = PhysicsModule::RigidBodyType::Dynamic);
    }
}

