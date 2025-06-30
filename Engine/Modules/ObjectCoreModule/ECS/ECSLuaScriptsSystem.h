#pragma once 
#include "../../LuaScriptModule/LuaScriptModule.h"
#include <flecs.h>

struct ScriptComponent {
	std::string scriptPath;
	std::optional<sol::table> script_table;
	std::optional<std::shared_ptr<ScriptModule::LuaScriptModule>> scriptModule;
	void initialize() {
		if (!script_table && !scriptPath.empty()) {
			sol::object result = scriptModule.value()->getLuaState().script_file(scriptPath, sol::script_throw_on_error);
			if (result.is<sol::table>()) {
				script_table = result.as<sol::table>();
			}
			else {
				script_table = scriptModule.value()->getLuaState().create_table();
			}
		}
	}

	void start(const flecs::entity& entity) {
		initialize();  
		if (script_table) {
			script_table.value()["entity"] = entity;
			sol::optional<sol::function> maybe_start = script_table.value()["Start"];
			if (maybe_start) {
				sol::function start_func = maybe_start.value();
				start_func();
			}
		}
	}
	void end() {
		if (script_table)
		{
			sol::optional<sol::function> maybe_end = script_table.value()["End"];
			if (maybe_end) {
				sol::function start_func = maybe_end.value();
				start_func();
			}
		}
	}

	void update(float deltaTime) {
		if (script_table)
		{
			sol::optional<sol::function> maybe_update = script_table.value()["Update"];

			if (maybe_update) {
				sol::function update_func = maybe_update.value();
				update_func(deltaTime); 
			}
		}
	}
};

class ECSluaScriptsSystem {
	ECSluaScriptsSystem() = default;
	ECSluaScriptsSystem(const ECSluaScriptsSystem&) = delete;
	ECSluaScriptsSystem& operator=(const ECSluaScriptsSystem&) = delete;

	std::optional<std::shared_ptr<ScriptModule::LuaScriptModule>> m_scriptModule;
public:
	static ECSluaScriptsSystem& getInstance() {
		static ECSluaScriptsSystem system;
		return system;
	}

	void registerSystem(flecs::world& world, std::shared_ptr<ScriptModule::LuaScriptModule> scriptModule) {
		m_scriptModule = scriptModule;
	world.system<ScriptComponent>()
			.kind(flecs::OnUpdate)
			.each([](flecs::iter& it, size_t, ScriptComponent& script) {
			script.update(it.delta_time());
		});
	}

	void startSystem(flecs::world& world) 
	{
		auto query = world.query<ScriptComponent>();
		query.each([this](const flecs::entity& entity, ScriptComponent& script) 
		{
				script.scriptModule = m_scriptModule;
				script.start(entity);
		});
	}

	void stopSystem(flecs::world& world) 
	{
		auto query = world.query<ScriptComponent>();
		query.each([](const flecs::entity& entity, ScriptComponent& script) 
		{
				script.end();
		});
	}
};
