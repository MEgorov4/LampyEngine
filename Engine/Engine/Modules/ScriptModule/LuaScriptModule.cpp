#include "LuaScriptModule.h"

#include "Registers/AudioRegister.h"
#include "Registers/EventRegister.h"
#include "Registers/FoundationRegister.h"
#include "Registers/ImGUIRegister.h"
#include "Registers/InputRegister.h"
#include "Registers/MathRegister.h"
#include "Registers/NetworkReceiveRegister.h"
#include "Registers/NetworkSendRegister.h"
#include "Registers/ECS/ECSWorldRegister.h"
#include "Registers/ECS/EntityRegister.h"
#include "Registers/ECS/LightRegister.h"
#include "Registers/ECS/PhysicsRegister.h"
#include "Registers/ECS/RenderComponentsRegister.h"
#include "Registers/ECS/TransformRegister.h"

#include <Modules/AudioModule/AudioModule.h>
#include <Modules/InputModule/InputModule.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ObjectCoreModule/ECS/EntityWorld.h>
#include <Modules/ResourceModule/Asset/AssetManager.h>
#include <Modules/ResourceModule/ResourceManager.h>

#include <format>
#include <stdexcept>

namespace ScriptModule
{
namespace
{
constexpr std::string_view kDevScriptPrefix = "Scripts/Dev";

class InputScriptServiceImpl final : public IInputScriptService
{
  public:
    explicit InputScriptServiceImpl(InputModule::InputModule* input) : m_input(input) {}

    EngineCore::Foundation::Event<SDL_KeyboardEvent>& keyboardEvent() override { return m_input->OnKeyboardEvent; }

  private:
    InputModule::InputModule* m_input;
};

class AudioScriptServiceImpl final : public IAudioScriptService
{
  public:
    explicit AudioScriptServiceImpl(AudioModule::AudioModule* audio) : m_audio(audio) {}

    void playSoundAsync() override
    {
        if (m_audio)
            m_audio->playSoundAsync();
    }

  private:
    AudioModule::AudioModule* m_audio;
};

class ECSWorldScriptServiceImpl final : public IECSWorldScriptService
{
  public:
    explicit ECSWorldScriptServiceImpl(ECSModule::ECSModule* ecs) : m_ecs(ecs) {}

    EntityWorld* currentWorld() override
    {
        return m_ecs ? m_ecs->getCurrentWorld() : nullptr;
    }

  private:
    ECSModule::ECSModule* m_ecs;
};

SandboxRule Deny(std::string table, std::string member)
{
    return SandboxRule{std::move(table), std::move(member)};
}
} // namespace

void LuaScriptModule::startup()
{
    cacheDependencies();
    initializeServices();
    initializeRegisters();
    buildProfiles();
    initializeVMs();
    initializeExecutors();
    initializeDevScripts();
    connectAssetEvents();
}

void LuaScriptModule::shutdown()
{
    disconnectAssetEvents();
    if (m_devScriptManager && m_vmManager.devVM())
    {
        m_devScriptManager->clear();
    }
    m_runtimeExecutor.reset();
    m_serverExecutor.reset();
    m_devScriptManager.reset();
    m_vmManager.shutdownAll();
    m_registerFactories.clear();
    m_profiles.clear();

    m_inputService.reset();
    m_audioService.reset();
    m_ecsService.reset();
    m_inputModule      = nullptr;
    m_audioModule      = nullptr;
    m_ecsModule        = nullptr;
    m_resourceManager  = nullptr;
    m_assetManager     = nullptr;
}

void LuaScriptModule::processCommand(const std::string& command)
{
    ScriptVM* target = m_vmManager.devVM();
    if (!target)
        target = m_vmManager.runtimeVM();
    if (!target)
        return;

    target->runString(command);
}

sol::state& LuaScriptModule::getLuaState()
{
    if (auto* runtime = m_vmManager.runtimeVM())
        return runtime->state();
    if (auto* dev = m_vmManager.devVM())
        return dev->state();
    if (auto* editor = m_vmManager.editorVM())
        return editor->state();
    throw std::runtime_error("No Script VM available");
}

ScriptVM* LuaScriptModule::getRuntimeVM() noexcept
{
    return m_vmManager.runtimeVM();
}

bool LuaScriptModule::hasDevScript(std::string_view name) const noexcept
{
    return m_devScriptManager && m_devScriptManager->hasScript(name);
}

std::vector<std::string> LuaScriptModule::listDevScripts() const
{
    if (!m_devScriptManager)
        return {};
    return m_devScriptManager->listScripts();
}

bool LuaScriptModule::reloadDevScript(std::string_view name)
{
    if (!m_devScriptManager || !m_vmManager.devVM())
        return false;
    return m_devScriptManager->reloadScript(*m_vmManager.devVM(), name);
}

void LuaScriptModule::cacheDependencies()
{
    m_inputModule     = GCM(InputModule::InputModule);
    m_audioModule     = GCM(AudioModule::AudioModule);
    m_ecsModule       = GCM(ECSModule::ECSModule);
    m_resourceManager = GCM(ResourceModule::ResourceManager);
    m_assetManager    = GCM(ResourceModule::AssetManager);
}

void LuaScriptModule::initializeServices()
{
    if (m_inputModule)
        m_inputService = std::make_unique<InputScriptServiceImpl>(m_inputModule);
    if (m_audioModule)
        m_audioService = std::make_unique<AudioScriptServiceImpl>(m_audioModule);
    if (m_ecsModule)
        m_ecsService = std::make_unique<ECSWorldScriptServiceImpl>(m_ecsModule);
}

void LuaScriptModule::initializeRegisters()
{
    m_registerFactories.clear();
    m_registerFactories["Foundation"]        = [] { return std::make_shared<FoundationRegister>(); };
    m_registerFactories["Events"]            = [] { return std::make_shared<EventRegister>(); };
    m_registerFactories["Math"]              = [] { return std::make_shared<MathRegister>(); };
    m_registerFactories["Transform"]         = [] { return std::make_shared<TransformRegister>(); };
    m_registerFactories["RenderComponents"]  = [] { return std::make_shared<RenderComponentsRegister>(); };
    m_registerFactories["Lights"]            = [] { return std::make_shared<LightRegister>(); };
    m_registerFactories["Physics"]           = [] { return std::make_shared<PhysicsRegister>(); };
    m_registerFactories["ImGui"]             = [] { return std::make_shared<ImGUIRegister>(); };
    m_registerFactories["NetworkSend"]       = [] { return std::make_shared<NetworkSendRegister>(); };
    m_registerFactories["NetworkReceive"]    = [] { return std::make_shared<NetworkReceiveRegister>(); };

    if (m_inputService)
    {
        auto* service = m_inputService.get();
        m_registerFactories["Input"] = [service]() { return std::make_shared<InputRegister>(service); };
    }
    if (m_audioService)
    {
        auto* service = m_audioService.get();
        m_registerFactories["Audio"] = [service]() { return std::make_shared<AudioRegister>(service); };
    }
    if (m_ecsService)
    {
        auto* service = m_ecsService.get();
        m_registerFactories["ECSWorld"] = [service]() { return std::make_shared<ECSWorldRegister>(service); };
        m_registerFactories["Entity"]   = [service]() { return std::make_shared<EntityRegister>(service); };
    }
}

void LuaScriptModule::buildProfiles()
{
    auto reg = [this](const std::string& name)
    { return registerFactory(name); };

    ScriptVMProfile runtime;
    runtime.name = "Runtime";
    runtime.libraries = {sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table};
    runtime.restrictions = {Deny("os", "execute"), Deny("io", "open"), Deny("debug", "traceback")};
    runtime.registerFactories = {
        reg("Foundation"),
        reg("Events"),
        reg("Math"),
        reg("ECSWorld"),
        reg("Entity"),
        reg("Transform"),
        reg("RenderComponents"),
        reg("Lights"),
        reg("Physics"),
        reg("Input"),
        reg("Audio")};
    m_profiles[ScriptVMType::Runtime] = runtime;

    ScriptVMProfile editor;
    editor.name = "Editor";
    editor.libraries = {sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::package};
    editor.registerFactories = {
        reg("Foundation"),
        reg("Events"),
        reg("Math"),
        reg("ImGui")};
    m_profiles[ScriptVMType::Editor] = editor;

    ScriptVMProfile dev;
    dev.name = "Dev";
    dev.libraries = {sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::package, sol::lib::os,
                     sol::lib::io, sol::lib::debug};
    dev.autoExecOnLoad = true;
    dev.registerFactories = {
        reg("Foundation"), reg("Events"), reg("Math"), reg("ECSWorld"), reg("Entity"), reg("Transform"),
        reg("RenderComponents"), reg("Lights"), reg("Physics"), reg("Input"), reg("Audio"), reg("ImGui"),
        reg("NetworkSend"), reg("NetworkReceive")};
    m_profiles[ScriptVMType::Dev] = dev;

    ScriptVMProfile client;
    client.name       = "Client";
    client.libraries  = {sol::lib::base, sol::lib::math, sol::lib::string};
    client.restrictions = {Deny("os", "execute"), Deny("io", "open"), Deny("package", "loadlib"), Deny("debug", "traceback")};
    client.registerFactories = {
        reg("Foundation"),
        reg("Events"),
        reg("Math"),
        reg("NetworkSend")};
    m_profiles[ScriptVMType::Client] = client;

    ScriptVMProfile server;
    server.name = "Server";
    server.libraries = {sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table};
    server.restrictions = {Deny("os", "execute"), Deny("debug", "traceback")};
    server.registerFactories = {
        reg("Foundation"),
        reg("Events"),
        reg("Math"),
        reg("ECSWorld"),
        reg("Entity"),
        reg("Physics"),
        reg("NetworkReceive")};
    m_profiles[ScriptVMType::Server] = server;

    ScriptVMProfile tools;
    tools.name      = "Tools";
    tools.libraries = {sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table};
    tools.registerFactories = {
        reg("Foundation"),
        reg("Events"),
        reg("Math"),
        reg("ImGui")};
    m_profiles[ScriptVMType::Tools] = tools;
}

void LuaScriptModule::initializeVMs()
{
    m_vmManager.initializeAll(m_profiles);
}

void LuaScriptModule::initializeExecutors()
{
    if (auto* runtime = m_vmManager.runtimeVM())
    {
        m_runtimeExecutor = std::make_unique<ScriptExecutor>("RuntimeExecutor");
        m_runtimeExecutor->attachVM(runtime);
    }

    if (auto* server = m_vmManager.serverVM())
    {
        m_serverExecutor = std::make_unique<ScriptExecutor>("ServerExecutor");
        m_serverExecutor->attachVM(server);
    }
}

void LuaScriptModule::initializeDevScripts()
{
    if (!m_resourceManager || !m_assetManager || !m_vmManager.devVM())
        return;

    m_devScriptManager = std::make_unique<DevScriptManager>(m_resourceManager, m_assetManager);
    m_devScriptManager->setAutoExecEnabled(true);
    m_devScriptManager->loadAll(*m_vmManager.devVM());
}

void LuaScriptModule::connectAssetEvents()
{
    if (!m_assetManager || !m_devScriptManager || !m_vmManager.devVM())
        return;

    m_assetImportedSubscription = m_assetManager->OnAssetImported.subscribe(
        [this](const ResourceModule::AssetInfo& info)
        {
            if (info.type != ResourceModule::AssetType::Script)
                return;

            if (info.sourcePath.empty())
                return;

            if (info.sourcePath.rfind(kDevScriptPrefix, 0) != 0)
                return;

            if (!m_devScriptManager->reloadScript(*m_vmManager.devVM(), info.guid))
            {
                m_devScriptManager->loadScript(*m_vmManager.devVM(), info.guid, info.sourcePath);
            }
        });
}

void LuaScriptModule::disconnectAssetEvents()
{
    m_assetImportedSubscription.unsubscribe();
}

std::function<std::shared_ptr<IScriptRegister>()> LuaScriptModule::registerFactory(const std::string& name) const
{
    auto it = m_registerFactories.find(name);
    if (it == m_registerFactories.end())
        return {};
    return it->second;
}
} // namespace ScriptModule
