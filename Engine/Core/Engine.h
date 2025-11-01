#pragma once

#include <EngineMinimal.h>

namespace InputModule
{
class InputModule;
}

namespace ECSModule
{
class ECSModule;
}

namespace PhysicsModule
{
class PhysicsModule;
}

namespace RenderModule
{
class RenderModule;
}

namespace WindowModule
{
class WindowModule;
}

/// <summary>
/// Manages the initialization, execution, and shutdown of the game engine.
/// </summary>
class Engine
{
    InputModule::InputModule *m_inputModule;
    WindowModule::WindowModule *m_windowModule;
    RenderModule::RenderModule *m_renderModule;
    PhysicsModule::PhysicsModule *m_physicsModule;
    ECSModule::ECSModule *m_ecsModule;

    std::unique_ptr<IEngineContext> m_engineContext; ///< Unique pointer to the engine context.
    std::unique_ptr<ContextLocator> m_contextLocator;

  public:
    Engine() = default;
    ~Engine() = default;
    Engine(const Engine &app) = delete;
    Engine(Engine &&app) = delete;
    Engine &operator=(const Engine &rhs) = delete;
    Engine &operator=(Engine &&rhs) = delete;

    /// <summary>
    /// Runs the engine, handling startup, execution, and shutdown processes.
    /// </summary>
    void run();

  private:
    void startupMajor();
    void startupMinor();
    void collectRuntimeModules();
    void shutdown();
    void engineTick();
};
