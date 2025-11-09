#pragma once

#include <EngineMinimal.h>
#include <Modules/ImGuiModule/GUIObject.h>
#include <vector>
#include <memory>
#include <string>

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

namespace ImGUIModule
{
class ImGUIModule;
}

class EditorGUIModule : public IModule
{
    ImGUIModule::ImGUIModule *m_imGuiModule;
    ProjectModule::ProjectModule *m_projectModule;
    FilesystemModule::FilesystemModule *m_filesystemModule;
    ECSModule::ECSModule *m_ecsModule;
    RenderModule::RenderModule *m_renderModule;
    InputModule::InputModule *m_inputModule;
    ScriptModule::LuaScriptModule *m_luaScriptModule;

    std::vector<std::weak_ptr<ImGUIModule::GUIObject>> m_windowObjects;
    std::vector<std::string> m_windowNames;

  public:
    void startup() override;
    void shutdown() override;

    void render(float deltaTime) const;

    // Window management
    const std::vector<std::weak_ptr<ImGUIModule::GUIObject>>& getWindows() const { return m_windowObjects; }
    const std::vector<std::string>& getWindowNames() const { return m_windowNames; }
};
