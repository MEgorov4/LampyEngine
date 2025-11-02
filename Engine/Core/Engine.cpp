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
#include <Modules/TimeModule/TimeModule.h>
#include <Modules/WindowModule/Window.h>
#include <Modules/WindowModule/WindowModule.h>

void Engine::run()
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

void Engine::startupMinor()
{
    ZoneScopedN("Engine::startupMinor");

    LT_LOG(LogVerbosity::Info, "Engine", "StartupMinor");
    Core::StartupAll();

    m_contextLocator = std::make_unique<ContextLocator>(Core::Locator());
    Context::SetLocator(m_contextLocator.get());

    m_engineContext = std::make_unique<Editor>();
    m_engineContext->startupMinor(*m_contextLocator);
}

void Engine::collectRuntimeModules()
{
    ZoneScopedN("Engine::collectRuntimeModules");

    m_inputModule = GCM(InputModule::InputModule);
    m_windowModule = GCM(WindowModule::WindowModule);
    m_renderModule = GCM(RenderModule::RenderModule);
    m_physicsModule = GCM(PhysicsModule::PhysicsModule);
    m_ecsModule = GCM(ECSModule::ECSModule);
    auto *assetManager = GCM(ResourceModule::AssetManager);
}

void Engine::startupMajor()
{
    ZoneScopedN("Engine::startupRuntimeModules");

    LT_LOG(LogVerbosity::Info, "Engine", "StartupMajor");

    using namespace EngineCore::Foundation;
    namespace fs = std::filesystem;

    auto *projectModule = GCXM(ProjectModule::ProjectModule);
    ProjectModule::ProjectConfig &cfg = projectModule->getProjectConfig();

    // ���� �������
    std::string projectRoot = cfg.getProjectPath();
    std::string projectResources = cfg.getResourcesPath();
    std::string projectBuild = cfg.getBuildPath();
    std::string projectCache = projectBuild + "/Cache";
    std::string projectContent = projectBuild + "/Content";
    std::string projectDb = Fs::absolutePath(projectRoot + "/Project.assetdb.json");

    // ���� ������ � ��������� ������������ ������� exe-����������
    fs::path exeDir = fs::current_path(); // build/Engine/Core/[Debug|Release]
    fs::path engineResources = fs::absolute(exeDir / "../../../build/Engine/Resources");

    // �����������, ��� ��� ������ ���������� ����������
    Fs::createDirectory(projectResources);
    Fs::createDirectory(projectCache);
    Fs::createDirectory(projectContent);

    // ������ � ������������� AssetManager
    auto assetMgr = std::make_shared<ResourceModule::AssetManager>();
    assetMgr->setEngineResourcesRoot(engineResources);
    assetMgr->setProjectResourcesRoot(projectResources);
    assetMgr->setCacheRoot(projectCache);
    assetMgr->setDatabasePath(projectDb);

    LT_LOG(LogVerbosity::Info, "Editor", "AssetManager initialized");
    LT_LOG(LogVerbosity::Info, "Editor", "EngineResources: " + engineResources.string());
    LT_LOG(LogVerbosity::Info, "Editor", "ProjectResources: " + projectResources);

    Core::Register(assetMgr, 5);

    // Регистрируем TimeModule с самым низким приоритетом, чтобы он был доступен другим модулям
    Core::Register(std::make_shared<TimeModule::TimeModule>(), 1);

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
    ZoneScopedN("Engine::shutdown");

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

            // Обновляем TimeModule (он может использовать переданный deltaTime или вычислить сам)
            {
                ZoneScopedN("Tick/TimeTick");
                auto *timeModule = GCM(TimeModule::TimeModule);
                timeModule->tick(deltaTime);
                // Используем deltaTime из TimeModule для согласованности
                deltaTime = timeModule->getDeltaTime();
            }

            {
                ZoneScopedN("Tick/PhysicsTick");
                m_physicsModule->tick(deltaTime);
            }

            {
                ZoneScopedN("Tick/ECSTick");
                m_ecsModule->ecsTick(deltaTime);
            }

            {
                ZoneScopedN("Tick/ContextTick");
                m_engineContext->tick(deltaTime);
            }
            {
                ZoneScopedN("Tick/Render");
                m_renderModule->getRenderer()->render();
                m_engineContext->render();
            }
            m_windowModule->getWindow()->swapWindow();
            // endFrame already called in IRenderer::render()

            TracyMessage("EFrame", 6);
        }
    }
    m_renderModule->getRenderer()->waitIdle();
}
