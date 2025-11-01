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
#include <Modules/ResourceModule/Asset/AssetManager.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ShaderCompilerModule/ShaderCompiler.h>
#include <Modules/WindowModule/Window.h>
#include <Modules/WindowModule/WindowModule.h>

void Engine::run()
{
    LT_PROFILE_ZONE("Engine::run");

    LT_LOGE("BEBE", Fs::currentPath());
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
    LT_PROFILE_ZONE("Engine::startupMinor");

    LT_LOG(LogVerbosity::Info, "Engine", "StartupMinor");
    Core::StartupAll();

    m_contextLocator = std::make_unique<ContextLocator>(Core::Locator());
    Context::SetLocator(m_contextLocator.get());

    m_engineContext = std::make_unique<Editor>();
    m_engineContext->startupMinor(*m_contextLocator);
}

void Engine::collectRuntimeModules()
{
    LT_PROFILE_ZONE("Engine::collectRuntimeModules");

    m_inputModule = GCM(InputModule::InputModule);
    m_windowModule = GCM(WindowModule::WindowModule);
    m_renderModule = GCM(RenderModule::RenderModule);
    m_physicsModule = GCM(PhysicsModule::PhysicsModule);
    m_ecsModule = GCM(ECSModule::ECSModule);
    auto *assetManager = GCM(ResourceModule::AssetManager);
}

void Engine::startupMajor()
{
    LT_PROFILE_ZONE("Engine::startupRuntimeModules");

    LT_LOG(LogVerbosity::Info, "Engine", "StartupMajor");

    using namespace EngineCore::Foundation;
    namespace fs = std::filesystem;

    auto *projectModule = GCXM(ProjectModule::ProjectModule);
    ProjectModule::ProjectConfig &cfg = projectModule->getProjectConfig();

    // ѕути проекта
    std::string projectRoot = cfg.getProjectPath();
    std::string projectResources = cfg.getResourcesPath();
    std::string projectBuild = cfg.getBuildPath();
    std::string projectCache = projectBuild + "/Cache";
    std::string projectContent = projectBuild + "/Content";
    std::string projectDb = Fs::absolutePath(projectRoot + "/Project.assetdb.json");

    // ѕути движка Ч вычисл€ем относительно текущей exe-директории
    fs::path exeDir = fs::current_path(); // build/Engine/Core/[Debug|Release]
    fs::path engineResources = fs::absolute(exeDir / "../../../build/Engine/Resources");

    // √арантируем, что все нужные директории существуют
    Fs::createDirectory(projectResources);
    Fs::createDirectory(projectCache);
    Fs::createDirectory(projectContent);

    // —оздаЄм и конфигурируем AssetManager
    auto assetMgr = std::make_shared<ResourceModule::AssetManager>();
    assetMgr->setEngineResourcesRoot(engineResources);
    assetMgr->setProjectResourcesRoot(projectResources);
    assetMgr->setCacheRoot(projectCache);
    assetMgr->setDatabasePath(projectDb);

    LT_LOG(LogVerbosity::Info, "Editor", "AssetManager initialized");
    LT_LOG(LogVerbosity::Info, "Editor", "EngineResources: " + engineResources.string());
    LT_LOG(LogVerbosity::Info, "Editor", "ProjectResources: " + projectResources);

    Core::Register(assetMgr, 5);

    auto resourceManager = std::make_shared<ResourceModule::ResourceManager>();
    resourceManager->setDatabase(&assetMgr->getDatabase());
    ResourceModule::AssetRegistryAccessor::Set(&assetMgr->getDatabase());
    resourceManager->setEngineResourcesRoot(engineResources.string());
    resourceManager->setProjectResourcesRoot(projectResources);

    Core::Register(std::make_shared<ShaderCompiler::ShaderCompiler>(), 10);
    Core::Register(resourceManager, 15);

    Core::Register(std::make_shared<InputModule::InputModule>(), 20);
    Core::Register(std::make_shared<WindowModule::WindowModule>(), 25);
    Core::Register(std::make_shared<AudioModule::AudioModule>(), 30);
    Core::Register(std::make_shared<ScriptModule::LuaScriptModule>(), 35);
    Core::Register(std::make_shared<RenderModule::RenderModule>(), 40);
    Core::Register(std::make_shared<ECSModule::ECSModule>(), 45);
    Core::Register(std::make_shared<PhysicsModule::PhysicsModule>(), 50);
    Core::StartupAll();
    ECSModule::ECSModule m_ecsModule;

    m_engineContext->startupMajor(*m_contextLocator);
}

void Engine::shutdown()
{
    LT_PROFILE_ZONE("Engine::shutdown");

    LT_LOG(LogVerbosity::Info, "Engine", "Shutdown");
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

    WindowModule::Window *window = GCM(WindowModule::WindowModule)->getWindow();

    while (!window->shouldClose())
    {
        LT_PROFILE_FRAME_BEGIN();
        LT_PROFILE_MARK("BFrame");

        LT_PROFILE_SCOPE("Tick");
        auto now = clock::now();
        double rawDt = std::chrono::duration<double>(now - prevTime).count();
        prevTime = now;

        rawDt = std::clamp(rawDt, 1e-6, 0.25);

        smothedDt += alpha * (rawDt - smothedDt);
        float deltaTime = static_cast<float>(smothedDt);

        window->pollEvents();

        LT_PROFILE_ZONE("Tick/PhysicsTick");
        m_physicsModule->tick(deltaTime);
        LT_PROFILE_ZONE("Tick/ECSTick");
        m_ecsModule->ecsTick(deltaTime);

        LT_PROFILE_ZONE("Tick/ContextTick");
        m_engineContext->tick(deltaTime);
        LT_PROFILE_ZONE("Tick/Render");
        m_renderModule->getRenderer()->render();
        m_windowModule->getWindow()->swapWindow();

        LT_PROFILE_MARK("EFrame");
        LT_PROFILE_FRAME_END();
    }
    m_renderModule->getRenderer()->waitIdle();
}
