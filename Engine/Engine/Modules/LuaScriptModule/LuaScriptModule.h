#pragma once

#include "../../Core/IModule.h"
#include "../../Foundation/Event/Event.h"

#include <sol/sol.hpp>

#include "IScriptRegister.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../ResourceModule/Asset/AssetID.h"
#include "../../Foundation/Event/Event.h"
#include "../ResourceModule/Asset/AssetInfo.h"
#include "../ResourceModule/Asset/AssetInfo.h"

namespace ECSModule
{
    class ECSModule;
}

namespace AudioModule
{
    class AudioModule;
}

namespace InputModule
{
    class InputModule;
}


namespace sol
{
    class state;
}

namespace ScriptModule
{
    class LuaScriptModule : public EngineCore::Base::IModule
    {
        InputModule::InputModule* m_inputModule;
        AudioModule::AudioModule* m_audioModule;
        ECSModule::ECSModule* m_ecsModule;
        
        sol::state m_luaState;
        
        struct DevScriptInstance
        {
            ResourceModule::AssetID id;
            std::string name;
            sol::environment environment;
            sol::table exports;
        };

        std::unordered_map<std::string, DevScriptInstance> m_devScripts;
        std::vector<std::unique_ptr<IScriptRegister>> m_registers;
        EngineCore::Foundation::Event<const ResourceModule::AssetInfo&>::Subscription m_assetImportedSubscription;
        EngineCore::Foundation::Event<const ResourceModule::AssetInfo&>::Subscription m_assetReloadSubscription;

    public:
        void startup() override;
        void shutdown() override;

        void processCommand(const std::string& command);

        sol::state& getLuaState()
        {
            return m_luaState;
        }

        bool hasDevScript(std::string_view name) const noexcept;
        std::vector<std::string> listDevScripts() const;
        bool reloadDevScript(std::string_view name);

        template <typename... Args>
        sol::protected_function_result callDevScript(std::string_view name, std::string_view function, Args&&... args)
        {
            std::string key{name};
            auto it = m_devScripts.find(key);
            if (it == m_devScripts.end())
            {
                LT_LOGE("ScriptModule", "Dev script not found: " + key);
            }

            std::string functionName{function};

            sol::object target = it->second.exports.get<sol::object>(functionName);
            if (!target.valid() || target.get_type() != sol::type::function)
            {
                target = it->second.environment.get<sol::object>(functionName);
            }

            if (!target.valid() || target.get_type() != sol::type::function)
            {
                LT_LOGE("ScriptModule", "Function not found in dev script: " + functionName);
            }

            sol::protected_function func = target;
            return func(std::forward<Args>(args)...);
        }

        InputModule::InputModule* getInputModule() const noexcept { return m_inputModule; }
        AudioModule::AudioModule* getAudioModule() const noexcept { return m_audioModule; }
        ECSModule::ECSModule* getECSModule() const noexcept { return m_ecsModule; }

    private:
        void initializeRegisters();

        void loadDevScriptsFromDatabase();
        bool loadDevScript(const ResourceModule::AssetID& id, std::string_view key);
        void clearDevScripts();
    };
}
