#pragma once

#include <EngineMinimal.h>

#include <sol/sol.hpp>


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
    class LuaScriptModule : public IModule
    {
        InputModule::InputModule* m_inputModule;
        AudioModule::AudioModule* m_audioModule;
        ECSModule::ECSModule* m_ecsModule;
        
        sol::state m_luaState;

    public:
        void startup() override;
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
