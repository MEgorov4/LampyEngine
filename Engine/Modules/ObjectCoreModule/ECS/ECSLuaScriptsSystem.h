#pragma once 
#include "../../LuaScriptModule/LuaScriptModule.h"
#include <flecs.h>

struct Script {
	std::string script_path;
	sol::table script_table;

	void update(float deltaTime) {
		sol::optional<sol::function> maybe_update = script_table["Update"];
		if (maybe_update) {
			sol::function update_func = maybe_update.value();
			update_func(deltaTime); 
		}
	}
};

class ECSluaScriptsSystem {
	ECSluaScriptsSystem() = default;
	ECSluaScriptsSystem(const ECSluaScriptsSystem&) = delete;
	ECSluaScriptsSystem& operator=(const ECSluaScriptsSystem&) = delete;

public:
	static ECSluaScriptsSystem& getInstance() {
		static ECSluaScriptsSystem system;
		return system;
	}

	void registerSystem(flecs::world& world) {
	auto script_sys = world.system<Script>()
			.each([](flecs::iter& it, size_t, Script& script) {

			auto event = it.event();

			if (event == flecs::PreUpdate)
			{
				sol::object result = LuaScriptModule::getInstance().getLuaState().script_file(script.script_path, sol::script_pass_on_error);
				if (result.is<sol::table>()) 
				{
					script.script_table = result.as<sol::table>();
				}
				else 
				{
					script.script_table = LuaScriptModule::getInstance().getLuaState().create_table();
				}
			}
			if (event == flecs::OnUpdate)
			{
				script.update(it.delta_time());
			}});
			script_sys.add(flecs::OnUpdate);
			script_sys.add(flecs::PreUpdate);
	}
};
