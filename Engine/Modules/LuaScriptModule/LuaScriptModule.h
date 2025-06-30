#pragma once
#include <string>
#include <memory>
#include <sol/sol.hpp>

#include "../../EngineContext/IModule.h"
#include "../../EngineContext/ModuleRegistry.h"

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

namespace Logger
{
    class Logger;
}

namespace sol
{
    class state;
}

namespace ScriptModule
{
    class LuaScriptModule : public IModule
    {
        std::shared_ptr<Logger::Logger> m_logger;
        std::shared_ptr<InputModule::InputModule> m_inputModule;
        std::shared_ptr<AudioModule::AudioModule> m_audioModule;
        std::shared_ptr<ECSModule::ECSModule> m_ecsModule;
        
        sol::state m_luaState;

    public:
        void startup(const ModuleRegistry& registry) override;
        void shutdown() override;

        void processCommand(const std::string& command);

        sol::state& getLuaState()
        {
            return m_luaState;
        }

    private:
        void registerLogger();
        void registerEvent();
        void registerInputModuleEvents();
        void registerAudioModule();
        void registerMathTypes();
        void registerECSModule();
    };
}
