#include "Engine.h"

#include "../EngineContext/EngineContext.h"
#include "../Editor/Editor.h"

#include "../Modules/WindowModule/WindowModule.h"
#include "../Modules/WindowModule/Window.h"

#include "../Modules/InputModule/InputModule.h"
#include "../Modules/RenderModule/RenderModule.h"
#include "../Modules/FilesystemModule/FilesystemModule.h"
#include "../Modules/ProjectModule/ProjectModule.h"
#include "../Modules/RenderModule/IRenderer.h"
#include "../Modules/AudioModule/AudioModule.h"
#include "../Modules/ObjectCoreModule/ECS/ECSModule.h"
#include "../Modules/ShaderCompilerModule/ShaderCompiler.h"
#include "../Modules/LoggerModule/Logger.h"
#include "../Modules/LuaScriptModule/LuaScriptModule.h"
#include "../Modules/ResourceModule/ResourceManager.h"
#include "../Modules/PhysicsModule/PhysicsModule.h"

#include "EngineConfig.h"
#include "../Modules/EditorGuiModule/EditorGUIModule.h"
#include "../Modules/ImGuiModule/ImGuiModule.h"

void Engine::run()
{
    startup();
    engineTick();
    shutdown();
}

void Engine::startup()
{
    m_moduleManager = std::make_unique<ModuleManager>();
    m_logger = m_moduleManager->createModule<Logger::Logger>("Logger");
    m_moduleManager->createModule<FilesystemModule::FilesystemModule>("FilesystemModule");

    ContextCreate();
    ContextMinorInit();

    m_moduleManager->createModule<AudioModule::AudioModule>("AudioModule");
    m_inputModule = m_moduleManager->createModule<InputModule::InputModule>("InputModule");

    m_windowModule = m_moduleManager->createModule<WindowModule::WindowModule>("WindowModule");

    m_moduleManager->createModule<ShaderCompiler::ShaderCompiler>("ShaderCompiler");
    m_moduleManager->createModule<ResourceModule::ResourceManager>("ResourceManager");
    m_renderModule = m_moduleManager->createModule<RenderModule::RenderModule>("RenderModule");

    m_ecsModule = m_moduleManager->createModule<ECSModule::ECSModule>("ECSModule");
    m_moduleManager->createModule<ScriptModule::LuaScriptModule>("ScriptModule");
    m_physicsModule = m_moduleManager->createModule<PhysicsModule::PhysicsModule>("PhysicsModule");

    ContextMajorInit();

    m_moduleManager->startupAll();
}

void Engine::shutdown()
{
    m_engineContext->shutdown();
    m_moduleManager->shutdownAll();
}

void Engine::ContextCreate()
{
    m_engineContext = std::make_unique<Editor>();
}

void Engine::ContextMinorInit() const 
{
    m_engineContext->initMinor(m_moduleManager.get());
}

void Engine::ContextMajorInit() const
{
    m_engineContext->initMajor(m_moduleManager.get());
}

void Engine::engineTick()
{
    float deltaTime = 0.0f;

    WindowModule::Window* window = m_windowModule->getWindow();
    float lastTime = window->currentTimeInSeconds();
    m_logger->log(Logger::LogVerbosity::Info, "Last time: " + std::to_string(lastTime), "Engine");

    while (!window->shouldClose())
    {
        float currentTime = window->currentTimeInSeconds();
        m_logger->log(Logger::LogVerbosity::Info, "Current time: " + std::to_string(currentTime), "Engine");

        deltaTime = currentTime - lastTime;
        m_logger->log(Logger::LogVerbosity::Info, "Delta time: " + std::to_string(deltaTime), "Engine");
        lastTime = currentTime;

        window->pollEvents();

        m_physicsModule->tick(deltaTime);
        m_ecsModule->ecsTick(deltaTime);

        m_engineContext->tick(deltaTime);
        m_renderModule->getRenderer()->render();
    }
    m_renderModule->getRenderer()->waitIdle();
}
