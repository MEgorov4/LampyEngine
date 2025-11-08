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
class Application
{
    InputModule::InputModule* m_inputModule;
    WindowModule::WindowModule* m_windowModule;
    RenderModule::RenderModule* m_renderModule;
    PhysicsModule::PhysicsModule* m_physicsModule;
    ECSModule::ECSModule* m_ecsModule;

    std::unique_ptr<IEngineContext> m_engineContext; ///< Unique pointer to the engine context.
    std::unique_ptr<ContextLocator> m_contextLocator;

  public:
    Application()                                  = default;
    virtual ~Application()                         = default;
    Application(const Application& app)            = delete;
    Application(Application&& app)                 = delete;
    Application& operator=(const Application& rhs) = delete;
    Application& operator=(Application&& rhs)      = delete;

    /// <summary>
    /// Runs the engine, handling startup, execution, and shutdown processes.
    /// </summary>
    void run();

  protected:
    virtual void onStartupMinor(ContextLocator* locator) = 0;
    virtual void onStartupMajor(ContextLocator* locator)  = 0;
    virtual void onShutdown() = 0;
    virtual void render()     = 0;
    virtual void tick(float dt) = 0;

  private:
    void startupMajor();
    void startupMinor();
    void collectRuntimeModules();
    void shutdown();
    void engineTick();
};
