#include "Application.h"

// Enable global memory tracking for leak detection
#ifdef TRACY_ENABLE
#define ENABLE_GLOBAL_MEMORY_TRACKING
#include <Foundation/Profiler/GlobalMemoryTracking.h>
#endif

#include <Foundation/Memory/MemorySystem.h>

#include <Editor/Editor.h>
#include <Modules/AudioModule/AudioModule.h>
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
#include <Modules/TimeModule/TimeModule.h>
#include <Modules/WindowModule/Window.h>
#include <Modules/WindowModule/WindowModule.h>

void Application::run()
{
    ZoneScopedN("Engine::run");

    LT_LOG(LogVerbosity::Info, "Engine", "Run");
    startupMinor();
    startupMajor();
    collectRuntimeModules();
    LT_LOG(LogVerbosity::Info, "Engine", "Create engine tick");
    engineTick();
    shutdown();
}

void Application::startupMinor()
{
    ZoneScopedN("Engine::startupMinor");

    LT_LOG(LogVerbosity::Info, "Engine", "StartupMinor");

    // Initialize memory system first, before anything else
    using namespace EngineCore::Foundation;
    MemorySystem::startup(1024 * 1024 * 1024);

    m_contextLocator = std::make_unique<ContextLocator>(Core::Locator());
    Context::SetLocator(m_contextLocator.get());
    onStartupMinor(m_contextLocator.get());
}

void Application::collectRuntimeModules()
{
    ZoneScopedN("Engine::collectRuntimeModules");

    m_inputModule = GCM(InputModule::InputModule);
    m_windowModule = GCM(WindowModule::WindowModule);
    m_renderModule = GCM(RenderModule::RenderModule);
    m_physicsModule = GCM(PhysicsModule::PhysicsModule);
    m_ecsModule = GCM(ECSModule::ECSModule);
    m_assetManager = GCM(ResourceModule::AssetManager);
}

void Application::startupMajor()
{
    ZoneScopedN("Engine::startupRuntimeModules");

    LT_LOG(LogVerbosity::Info, "Engine", "StartupMajor");

    Core::Register(std::make_shared<JobSystem>(), 0);
    Core::Register(std::make_shared<TimeModule::TimeModule>(), 1);

    using namespace EngineCore::Foundation;
    namespace fs = std::filesystem;
    

    auto *projectModule = GCXM(ProjectModule::ProjectModule);
    ProjectModule::ProjectConfig &cfg = projectModule->getProjectConfig();

    std::string projectRoot = cfg.getProjectPath();
    std::string projectResources = cfg.getResourcesPath();
    std::string projectBuild = cfg.getBuildPath();
    std::string projectCache = projectBuild + "/Cache";
    std::string projectContent = projectBuild + "/Content";
    std::string projectDb = Fs::absolutePath(projectRoot + "/Project.assetdb.json");

    fs::path exeDir = fs::current_path(); 

    fs::path engineResources = fs::absolute(exeDir / "../Resources");
    if (!fs::exists(engineResources))
        engineResources = fs::absolute(exeDir / "../../Resources"); // TODO: FIX 
    LT_LOGE("Engine", "Exe path: " + exeDir.string());

    Fs::createDirectory(projectResources);
    Fs::createDirectory(projectCache);
    Fs::createDirectory(projectContent);

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
    Core::Register(std::make_shared<RenderModule::RenderModule>(), 40);
    Core::Register(std::make_shared<ImGUIModule::ImGUIModule>(), 45);
    Core::Register(std::make_shared<ECSModule::ECSModule>(), 50);
    Core::Register(std::make_shared<PhysicsModule::PhysicsModule>(), 55);
    Core::Register(std::make_shared<ScriptModule::LuaScriptModule>(), 60);
    Core::StartupAll();

    onStartupMajor(m_contextLocator.get());
}

void Application::shutdown()
{
    ZoneScopedN("Engine::shutdown");

    LT_LOG(LogVerbosity::Info, "Engine", "Shutdown");
    m_contextLocator->shutdownAll();
    Core::ShutdownAll();
    
    // Shutdown memory system last
    using namespace EngineCore::Foundation;
    MemorySystem::shutdown();
}

void Application::engineTick()
{
    using clock = std::chrono::steady_clock;
    auto prevTime = clock::now();
    double smothedDt = 1.0 / 60.0;
    const double alpha = 0.1;

    WindowModule::Window *window = GCM(WindowModule::WindowModule)->getWindow();

    while (!window->shouldClose())
    {
        FrameMark;
        TracyMessage("BFrame", 5);

        {
            ZoneScopedN("Tick");
            auto now = clock::now();
            double rawDt = std::chrono::duration<double>(now - prevTime).count();
            prevTime = now;

            rawDt = std::clamp(rawDt, 1e-6, 0.25);

            smothedDt += alpha * (rawDt - smothedDt);
            float deltaTime = static_cast<float>(smothedDt);

            window->pollEvents();

            if (m_assetManager)
            {
                ZoneScopedN("Tick/ProcessAssetChanges");
                m_assetManager->processFileChanges();
            }

            {
                ZoneScopedN("Tick/TimeTick");
                auto *timeModule = GCM(TimeModule::TimeModule);
                timeModule->tick(deltaTime);
                deltaTime = timeModule->getDeltaTime();
            }

            {
                ZoneScopedN("Tick/ECSTick");
                // ECS tick runs all systems in OnUpdate phase
                // SyncToPhysics runs (synchronizes ECS -> Physics)
                // SyncFromPhysics also runs (synchronizes Physics -> ECS from previous frame)
                m_ecsModule->ecsTick(deltaTime);
            }

            {
                ZoneScopedN("Tick/PhysicsTick");
                // Physics step runs after ECS tick
                // SyncFromPhysics will sync results on next frame
                m_physicsModule->tick(deltaTime);
            }

            {
                ZoneScopedN("Tick/ContextTick");
                tick(deltaTime);
            }
            {
                ZoneScopedN("Tick/Render");
                m_renderModule->getRenderer()->render();
            }
            {
                ZoneScopedN("Tick/ContextRender");
                render();
            }
            
            m_windowModule->getWindow()->swapWindow();

            // Reset frame allocator at end of frame
            MemorySystem::resetFrameAllocator();

            TracyMessage("EFrame", 6);
        }
    }
    m_renderModule->getRenderer()->waitIdle();
}
