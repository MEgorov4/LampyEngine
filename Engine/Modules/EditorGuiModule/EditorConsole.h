#pragma once
#include <string>
#include <list>
#include <memory>

#include "../ImGuiModule/GUIObject.h"

namespace ScriptModule
{
	class LuaScriptModule;
}

class GUIEditorConsole : public ImGuiModule::GUIObject
{
	std::shared_ptr<ScriptModule::LuaScriptModule> m_luaScriptModule;
public:
	GUIEditorConsole(const std::shared_ptr<ScriptModule::LuaScriptModule>& scriptModule);
	~GUIEditorConsole() override = default;

	void render() override;

private:
	void processCommand(const std::string& command) const;
};
