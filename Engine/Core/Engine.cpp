#include "Engine.h"

#include "EngineConfig.h"

#include <Editor/Editor.h>
#include <Modules/AudioModule/AudioModule.h>
#include <Modules/EditorGuiModule/EditorGUIModule.h>
#include <Modules/ImGuiModule/ImGuiModule.h>
#include <Modules/InputModule/InputModule.h>
#include <Modules/LuaScriptModule/LuaScriptModule.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/PhysicsModule/PhysicsModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/RenderModule/IRenderer.h>
#include <Modules/RenderModule/RenderModule.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ShaderCompilerModule/ShaderCompiler.h>
#include <Modules/WindowModule/Window.h>
#include <Modules/WindowModule/WindowModule.h>

void Engine::run()
{
    LT_LOG(LogVerbosity::Info, "Engine", "Run");
    startupMinor();
    startupMajor();
    collectRuntimeModules();
    LT_LOG(LogVerbosity::Info, "Engine", "Create engine tick");
    engineTick();
    shutdown();
}

void Engine::startupMinor()
{
    LT_LOG(LogVerbosity::Info, "Engine", "StartupMinor");
    Core::StartupAll();

    m_contextLocator = std::make_unique<ContextLocator>(Core::Locator());
    Context::SetLocator(m_contextLocator.get());

    m_engineContext = std::make_unique<Editor>();
    m_engineContext->initMinor(*m_contextLocator);
}

void Engine::collectRuntimeModules()
{
    m_inputModule   = GCM(InputModule::InputModule);
    m_windowModule  = GCM(WindowModule::WindowModule);
    m_renderModule  = GCM(RenderModule::RenderModule);
    m_physicsModule = GCM(PhysicsModule::PhysicsModule);
    m_ecsModule     = GCM(ECSModule::ECSModule);
}

void Engine::startupMajor()
{
    LT_LOG(LogVerbosity::Info, "Engine", "StartupMajor");
    Core::Register(std::make_shared<ShaderCompiler::ShaderCompiler>(), 10);
    Core::Register(std::make_shared<ResourceModule::ResourceManager>(), 15);
    Core::Register(std::make_shared<InputModule::InputModule>(), 20);
    Core::Register(std::make_shared<WindowModule::WindowModule>(), 25);
    Core::Register(std::make_shared<AudioModule::AudioModule>(), 30);
    Core::Register(std::make_shared<ScriptModule::LuaScriptModule>(), 35);
    Core::Register(std::make_shared<RenderModule::RenderModule>(), 40);
    Core::Register(std::make_shared<ECSModule::ECSModule>(), 45);
    Core::Register(std::make_shared<PhysicsModule::PhysicsModule>(), 50);
    Core::StartupAll();

    ECSModule::ECSModule m_ecsModule;

    m_engineContext->initMajor(*m_contextLocator);
}

void Engine::shutdown()
{
    LT_LOG(LogVerbosity::Info, "Engine", "Shutdown");
    m_engineContext->shutdown();
    m_contextLocator->shutdownAll();
    Core::ShutdownAll();
}

void Engine::engineTick()
{
    using clock        = std::chrono::steady_clock;
    auto prevTime      = clock::now();
    double smothedDt   = 1.0 / 60.0;
    const double alpha = 0.1;

    WindowModule::Window* window = GCM(WindowModule::WindowModule)->getWindow();

    while (!window->shouldClose())
    {
        auto now     = clock::now();
        double rawDt = std::chrono::duration<double>(now - prevTime).count();
        prevTime     = now;

        rawDt = std::clamp(rawDt, 1e-6, 0.25);

        smothedDt += alpha * (rawDt - smothedDt);
        float deltaTime = static_cast<float>(smothedDt);

        window->pollEvents();

        m_physicsModule->tick(deltaTime);
        m_ecsModule->ecsTick(deltaTime);

        m_engineContext->tick(deltaTime);
        m_renderModule->getRenderer()->render();
        m_windowModule->getWindow()->swapWindow();
    }
    m_renderModule->getRenderer()->waitIdle();
}
