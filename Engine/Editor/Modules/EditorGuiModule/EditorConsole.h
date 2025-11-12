#pragma once

#include <EngineMinimal.h>
#include <Modules/ImGuiModule/GUIObject.h>

namespace ScriptModule
{
class LuaScriptModule;
}

class GUIConsole : public ImGUIModule::GUIObject
{
    ScriptModule::LuaScriptModule* m_luaScriptModule;

  public:
    GUIConsole();
    ~GUIConsole() override = default;

    void render(float deltaTime) override;

  private:
    void processCommand(const std::string& command) const;
};
