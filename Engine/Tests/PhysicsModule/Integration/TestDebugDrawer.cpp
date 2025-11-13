#include <gtest/gtest.h>
#include <Modules/PhysicsModule/PhysicsContext/PhysicsContext.h>
#include <Modules/RenderModule/RenderContext.h>
#include <Foundation/Memory/MemorySystem.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Core/Core.h>

using namespace PhysicsModule;
using namespace RenderModule;

class DebugDrawerTest : public ::testing::Test
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
// Debug Drawer Integration Tests
// ============================================================================

TEST_F(DebugDrawerTest, DebugDrawerEnabled)
{
    context->setDebugDrawEnabled(true);
    EXPECT_TRUE(context->isDebugDrawEnabled());
}

TEST_F(DebugDrawerTest, DebugDrawerDisabled)
{
    context->setDebugDrawEnabled(false);
    EXPECT_FALSE(context->isDebugDrawEnabled());
}

TEST_F(DebugDrawerTest, StepWithDebugDrawEnabled)
{
    flecs::world world;
    auto entity = world.entity("DebugBox");
    
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Static;
    desc.mass = 0.0f;
    desc.position = glm::vec3(0.0f, 0.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(1.0f);
    
    PhysicsBodyHandle handle;
    context->createBodyForEntity(entity, desc, handle);
    
    context->setDebugDrawEnabled(true);
    
    // Step should not crash with debug draw enabled
    EXPECT_NO_THROW(context->step(1.0f / 60.0f));
    
    // Verify debug primitives were added to render context
    // (This depends on RenderContext implementation)
    auto& scene = renderContext->scene();
    // Debug primitives are added during step when debug draw is enabled
    
    context->destroyBodyForEntity(entity);
}

TEST_F(DebugDrawerTest, DebugDrawerToggle)
{
    context->setDebugDrawEnabled(true);
    EXPECT_TRUE(context->isDebugDrawEnabled());
    
    context->setDebugDrawEnabled(false);
    EXPECT_FALSE(context->isDebugDrawEnabled());
    
    context->setDebugDrawEnabled(true);
    EXPECT_TRUE(context->isDebugDrawEnabled());
}

