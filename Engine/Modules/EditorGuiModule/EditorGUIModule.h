#pragma once
#include <memory>

#include "../../EngineContext/IModule.h"
#include "../../EngineContext/ModuleRegistry.h"

namespace ScriptModule
{
    class LuaScriptModule;
}

namespace RenderModule
{
    class RenderModule;
}

namespace InputModule
{
    class InputModule;
}

namespace FilesystemModule
{
    class FilesystemModule;
}

namespace ECSModule
{
    class ECSModule;
}

namespace ProjectModule
{
    class ProjectModule;
}

namespace Logger
{
    class Logger;
}

namespace ImGuiModule
{
    class ImGuiModule;
}


class EditorGUIModule : public IModule
{
    std::shared_ptr<ImGuiModule::ImGuiModule> m_imGuiModule;
    std::shared_ptr<ProjectModule::ProjectModule> m_projectModule;
    std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
    std::shared_ptr<ECSModule::ECSModule> m_ecsModule;
    std::shared_ptr<RenderModule::RenderModule> m_renderModule;
    std::shared_ptr<InputModule::InputModule> m_inputModule;
    std::shared_ptr<ScriptModule::LuaScriptModule> m_luaScriptModule;
    
    std::shared_ptr<Logger::Logger> m_logger;
public:
    void startup(const ModuleRegistry& registry) override;
    void shutdown() override;
    
    void render() const;
};
