#include "Engine.h"

#include <Editor/Editor.h>

#include <Modules/WindowModule/WindowModule.h>
#include <Modules/WindowModule/Window.h>

#include <Modules/InputModule/InputModule.h>
#include <Modules/RenderModule/RenderModule.h>
#include <Modules/FilesystemModule/FilesystemModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/RenderModule/IRenderer.h>
#include <Modules/AudioModule/AudioModule.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ShaderCompilerModule/ShaderCompiler.h>
#include <Modules/LuaScriptModule/LuaScriptModule.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/PhysicsModule/PhysicsModule.h>
#include <Modules/EditorGuiModule/EditorGUIModule.h>
#include <Modules/ImGuiModule/ImGuiModule.h>

#include "EngineConfig.h"

void Engine::run()
{
    LT_LOG(LogVerbosity::Debug, "Engine", "run");
    startupMinor();
    startupMajor();

    LT_LOG(LogVerbosity::Debug, "Engine", "create engine tick");
    engineTick();
    shutdown();
}

void Engine::startupMinor()
{
    LT_LOG(LogVerbosity::Debug, "Engine", "startupMinor");
    Core::Register(std::make_shared<FilesystemModule::FilesystemModule>(), 5);
    Core::Register(std::make_shared<ProjectModule::ProjectModule>(), 10);
    Core::Register(std::make_shared<ResourceModule::ResourceManager>(), 15);
    Core::Register(std::make_shared<AudioModule::AudioModule>(), 20);
    Core::Register(std::make_shared<ScriptModule::LuaScriptModule>(), 25);
    Core::StartupAll();

    m_contextLocator = std::make_unique<ContextLocator>(Core::Locator());
    Context::SetLocator(m_contextLocator.get());

    m_engineContext = std::make_unique<Editor>();
    m_engineContext->initMinor(*m_contextLocator);
}

void Engine::startupMajor()
{
    LT_LOG(LogVerbosity::Debug, "Engine", "startupMajor");
    Core::Register(std::make_shared<WindowModule::WindowModule>(), 40);
    Core::Register(std::make_shared<InputModule::InputModule>(), 30);
    Core::Register(std::make_shared<ShaderCompiler::ShaderCompiler>(), 50);
    Core::Register(std::make_shared<RenderModule::RenderModule>(), 60);
    Core::Register(std::make_shared<ECSModule::ECSModule>(), 70);
    Core::Register(std::make_shared<PhysicsModule::PhysicsModule>(), 80);
    Core::StartupAll();

    m_engineContext->initMajor(*m_contextLocator);
}

void Engine::shutdown()
{
    LT_LOG(LogVerbosity::Debug, "Engine", "shutdown");
    m_engineContext->shutdown();
    m_contextLocator->shutdownAll();
    Core::ShutdownAll();
}

void Engine::engineTick()
{
    using clock = std::chrono::steady_clock;
    auto prevTime = clock::now();
    double smothedDt = 1.0 / 60.0;
    const double alpha = 0.1;
    

    WindowModule::Window* window = m_windowModule->getWindow();
    
    while (!window->shouldClose())
    {
        auto now = clock::now();
        double rawDt = std::chrono::duration<double>(now - prevTime).count();
        prevTime = now;

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
