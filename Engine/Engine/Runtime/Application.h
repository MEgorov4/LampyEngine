#pragma once

#include <EngineMinimal.h>
#include "ModuleConfigRegistry.h"

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

namespace UIModule
{
class UIModule;
}

namespace ResourceModule
{
class AssetManager;
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
    InputModule::InputModule* m_inputModule = nullptr;
    WindowModule::WindowModule* m_windowModule = nullptr;
    RenderModule::RenderModule* m_renderModule = nullptr;
    PhysicsModule::PhysicsModule* m_physicsModule = nullptr;
    ECSModule::ECSModule* m_ecsModule = nullptr;
    ResourceModule::AssetManager* m_assetManager = nullptr;

    std::unique_ptr<ContextLocator> m_contextLocator;
    ModuleConfigRegistry m_moduleConfigRegistry;

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
    virtual void configureModules(ModuleConfigRegistry& registry) {}

    ModuleConfigRegistry& moduleConfigRegistry()
    {
        return m_moduleConfigRegistry;
    }
    const ModuleConfigRegistry& moduleConfigRegistry() const
    {
        return m_moduleConfigRegistry;
    }

  private:
    void startupMajor();
    void startupMinor();
    void collectRuntimeModules();
    void shutdown();
    void engineTick();
};
