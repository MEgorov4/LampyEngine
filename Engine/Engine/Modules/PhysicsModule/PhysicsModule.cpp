#include "PhysicsModule.h"
#include "PhysicsLocator.h"
#include "PhysicsContext/PhysicsContext.h"
#include "../../Foundation/Event/EventBus.h"
#include "../../Core/CoreGlobal.h"
#include "../../Modules/RenderModule/RenderLocator.h"
#include "../../Modules/ObjectCoreModule/ECS/ECSModule.h"
#include "Systems/PhysicsStepSystem.h"
#include "Systems/SyncToPhysicsSystem.h"
#include "Systems/SyncFromPhysicsSystem.h"
#include "Systems/PhysicsRaycastSystem.h"

namespace PhysicsModule
{
    struct PhysicsModule::Impl
    {
        std::unique_ptr<PhysicsContext> context;
    };

    PhysicsModule::PhysicsModule()
        : m_impl(std::make_unique<Impl>())
    {
    }

    PhysicsModule::~PhysicsModule() = default;

    void PhysicsModule::startup()
    {
        ZoneScopedN("PhysicsModule::startup");
        LT_LOGI("PhysicsModule", "Startup");

        // Get RenderContext for debug drawing
        auto* renderContext = RenderModule::RenderLocator::Get();
        if (!renderContext)
        {
            LT_LOGW("PhysicsModule", "RenderContext not available, debug drawing disabled");
        }

        // Create PhysicsContext
        m_impl->context = std::make_unique<PhysicsContext>(renderContext);

        // Register in DI
        PhysicsLocator::Provide(m_impl->context.get());

        // Connect to EventBus
        m_impl->context->connectToEventBus(GCEB());

        // Register ECS systems
        // Systems will be registered when ECS world is available
        // This is typically done in ECSModule startup or when world is created

        LT_LOGI("PhysicsModule", "Startup complete");
    }

    void PhysicsModule::shutdown()
    {
        ZoneScopedN("PhysicsModule::shutdown");
        LT_LOGI("PhysicsModule", "Shutdown");

        PhysicsLocator::Reset();
        m_impl->context.reset();
        m_impl.reset();

        LT_LOGI("PhysicsModule", "Shutdown complete");
    }

    void PhysicsModule::tick(float dt) noexcept
    {
        // Physics step - called from main loop
        if (m_impl->context)
        {
            m_impl->context->step(dt);
        }
    }
}
