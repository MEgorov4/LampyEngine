#pragma once

#include "../../Core/IModule.h"
#include "../../Foundation/Event/Event.h"
#include "../../Foundation/Log/LoggerMacro.h"

#include "DevScriptManager.h"
#include "ScriptExecutor.h"
#include "ScriptServices.h"
#include "ScriptVMManager.h"
#include "ScriptVMProfile.h"

#include <Modules/ResourceModule/Asset/AssetInfo.h>

#include <sol/sol.hpp>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace AudioModule
{
class AudioModule;
}

namespace ECSModule
{
class ECSModule;
}

namespace InputModule
{
class InputModule;
}

namespace ResourceModule
{
class AssetManager;
class ResourceManager;
}

namespace ScriptModule
{
class LuaScriptModule : public EngineCore::Base::IModule
{
  public:
    void startup() override;
    void shutdown() override;

    void processCommand(const std::string& command);

    sol::state& getLuaState();
    ScriptVM* getRuntimeVM() noexcept;

    bool hasDevScript(std::string_view name) const noexcept;
    std::vector<std::string> listDevScripts() const;
    bool reloadDevScript(std::string_view name);

    template <typename... Args>
    bool callDevScript(std::string_view name, std::string_view function, Args&&... args)
    {
        if (!m_devScriptManager || !m_vmManager.devVM())
            return false;

        const sol::table* exports = m_devScriptManager->getExports(name);
        if (!exports)
        {
            LT_LOGE("ScriptModule", "Dev script not found: " + std::string(name));
            return false;
        }

        std::string functionName{function};
        sol::object target = exports->get<sol::object>(functionName);
        if (!target.valid() || target.get_type() != sol::type::function)
        {
            LT_LOGE("ScriptModule", "Function not found in dev script: " + functionName);
            return false;
        }

        sol::protected_function func = target;
        sol::protected_function_result result = func(std::forward<Args>(args)...);
        if (!result.valid())
        {
            sol::error err = result;
            LT_LOGE("ScriptModule", std::string("Dev script call failed: ") + err.what());
            return false;
        }
        return true;
    }

  private:
    void cacheDependencies();
    void initializeServices();
    void initializeRegisters();
    void buildProfiles();
    void initializeVMs();
    void initializeExecutors();
    void initializeDevScripts();
    void connectAssetEvents();
    void disconnectAssetEvents();

    std::function<std::shared_ptr<IScriptRegister>()> registerFactory(const std::string& name) const;

  private:
    InputModule::InputModule* m_inputModule{nullptr};
    AudioModule::AudioModule* m_audioModule{nullptr};
    ECSModule::ECSModule* m_ecsModule{nullptr};
    ResourceModule::ResourceManager* m_resourceManager{nullptr};
    ResourceModule::AssetManager* m_assetManager{nullptr};

    std::unique_ptr<IInputScriptService> m_inputService;
    std::unique_ptr<IAudioScriptService> m_audioService;
    std::unique_ptr<IECSWorldScriptService> m_ecsService;

    ScriptVMManager m_vmManager;
    ScriptVMProfileMap m_profiles;
    std::unique_ptr<DevScriptManager> m_devScriptManager;
    std::unique_ptr<ScriptExecutor> m_runtimeExecutor;
    std::unique_ptr<ScriptExecutor> m_serverExecutor;

    std::unordered_map<std::string, std::function<std::shared_ptr<IScriptRegister>()>> m_registerFactories;

    EngineCore::Foundation::Event<const ResourceModule::AssetInfo&>::Subscription m_assetImportedSubscription;
};
} // namespace ScriptModule
