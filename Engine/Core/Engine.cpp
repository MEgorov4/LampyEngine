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
    shutDownEngineContextObject();
    shutDownModules();
}

void Engine::startup()
{
    m_moduleManager = std::make_unique<ModuleManager>();
    m_moduleManager->createModule<Logger::Logger>("Logger");
    m_moduleManager->createModule<FilesystemModule::FilesystemModule>("FilesystemModule");

    ContextCreate();
    ContextMinorInit();

    m_moduleManager->createModule<AudioModule::AudioModule>("AudioModule");
    m_windowModule = m_moduleManager->createModule<WindowModule::WindowModule>("WindowModule");
    m_moduleManager->createModule<InputModule::InputModule>("InputModule");

    m_moduleManager->createModule<ShaderCompiler::ShaderCompiler>("ShaderCompiler");
    m_moduleManager->createModule<ResourceModule::ResourceManager>("ResourceManager");
    m_renderModule = m_moduleManager->createModule<RenderModule::RenderModule>("RenderModule");
    m_moduleManager->createModule<ImGuiModule::ImGuiModule>("ImGuiModule");

    m_ecsModule = m_moduleManager->createModule<ECSModule::ECSModule>("ECSModule");
    m_moduleManager->createModule<ScriptModule::LuaScriptModule>("ScriptModule");
    m_physicsModule = m_moduleManager->createModule<PhysicsModule>("PhysicsModule");

    ContextMajorInit();

    m_moduleManager->startupAll();
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

    while (!window->shouldClose())
    {
        float currentTime = window->currentTimeInSeconds();

        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        window->pollEvents();

        m_physicsModule->tick(deltaTime);
        m_ecsModule->ecsTick(deltaTime);

        m_engineContext->tick(deltaTime);
        m_renderModule->getRenderer()->render();
    }
    m_renderModule->getRenderer()->waitIdle();
}


void Engine::shutDownEngineContextObject()
{
    m_engineContext->shutdown();
}

void Engine::shutDownModules()
{
    m_moduleManager->shutdownAll();
}
