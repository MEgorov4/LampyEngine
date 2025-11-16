#pragma once 
#include "Modules/ScriptModule/LuaScriptModule.h"
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ResourceModule/Script.h>
#include <Foundation/Log/LoggerMacro.h>
#include <flecs.h>

#include <exception>
#include <format>
#include <utility>

namespace
{
constexpr std::string_view kScriptSystemCategory = "ECSLuaScriptsSystem";
}

struct ScriptComponent {
	ResourceModule::AssetID scriptID;

	ScriptModule::LuaScriptModule* scriptModule{nullptr};
	ResourceModule::ResourceManager* resourceManager{nullptr};

	void start(const flecs::entity& entity) {
		if (!ensureScriptLoaded(entity))
			return;
		invokeStart();
	}

	void end() {
		if (!m_scriptTable.valid())
			return;
		invokeFunction("OnEnd");
		resetState();
	}

	void update(const flecs::entity& entity, float deltaTime) {
		if (!ensureScriptLoaded(entity))
			return;
		invokeStart();
		invokeFunction("OnUpdate", deltaTime);
	}

  private:
	bool ensureScriptLoaded(const flecs::entity& entity) {
		if (scriptID.empty()) {
			if (m_scriptTable.valid())
				resetState();
			return false;
		}

		if (!scriptModule || !resourceManager)
			return false;

		if (!m_loadedScriptID.empty() && m_loadedScriptID == scriptID && m_scriptTable.valid()) {
			assignEntityContext(entity);
			return true;
		}

		auto scriptResource = resourceManager->load<ResourceModule::RScript>(scriptID);
		if (!scriptResource) {
			LT_LOGW(kScriptSystemCategory.data(), std::format("Failed to load script asset [{}]", scriptID.str()));
			resetState();
			return false;
		}

		sol::state* targetState = nullptr;
		sol::environment baseEnv;

		if (auto* runtimeVM = scriptModule->getRuntimeVM()) {
			targetState = &runtimeVM->state();
			baseEnv = runtimeVM->environment();
		}
		else {
			try {
				targetState = &scriptModule->getLuaState();
			}
			catch (const std::exception& ex) {
				LT_LOGE(kScriptSystemCategory.data(),
				        std::format("No Lua VM available to load script [{}]: {}", scriptID.str(), ex.what()));
				return false;
			}
			baseEnv = sol::environment(*targetState, sol::create, targetState->globals());
		}

		if (!targetState) {
			LT_LOGE(kScriptSystemCategory.data(),
			        std::format("Lua state unavailable for script [{}]", scriptID.str()));
			return false;
		}

		sol::load_result chunk = targetState->load(scriptResource->getSource());
		if (!chunk.valid()) {
			sol::error err = chunk;
			LT_LOGE(kScriptSystemCategory.data(),
			        std::format("Lua load error for script [{}]: {}", scriptID.str(), err.what()));
			resetState();
			return false;
		}

		sol::protected_function func = chunk;
		m_environment = sol::environment(*targetState, sol::create, baseEnv);
		sol::set_environment(m_environment, func);
		sol::protected_function_result exec = func();
		if (!exec.valid()) {
			sol::error err = exec;
			LT_LOGE(kScriptSystemCategory.data(),
			        std::format("Lua execution error for script [{}]: {}", scriptID.str(), err.what()));
			resetState();
			return false;
		}

		sol::table exports;
		if (exec.return_count() > 0) {
			sol::object ret = exec.get<sol::object>();
			if (ret.valid() && ret.get_type() == sol::type::table) {
				exports = ret.as<sol::table>();
			}
		}

		if (!exports.valid()) {
			exports = targetState->create_table();
		}

		m_scriptTable = exports;
		m_loadedScriptID = scriptID;
		m_started = false;
		assignEntityContext(entity);
		return true;
	}

	void assignEntityContext(const flecs::entity& entity) {
		if (m_scriptTable.valid()) {
			m_scriptTable["entity"] = entity;
		}
	}

	void resetState() {
		m_scriptTable = sol::table();
		m_environment = sol::environment();
		m_loadedScriptID = {};
		m_started = false;
	}

	void invokeStart() {
		if (m_started || !m_scriptTable.valid())
			return;
		invokeFunction("OnStart");
		m_started = true;
	}

	template <typename... Args>
	void invokeFunction(const char* functionName, Args&&... args) {
		if (!m_scriptTable.valid())
			return;

		sol::object candidate = m_scriptTable[functionName];
		if (!candidate.valid() || candidate.get_type() != sol::type::function)
			return;

		sol::protected_function func = candidate;
		sol::protected_function_result result = func(std::forward<Args>(args)...);
		if (!result.valid()) {
			sol::error err = result;
			LT_LOGE(kScriptSystemCategory.data(),
			        std::format("Script [{}] {} failed: {}", scriptID.str(), functionName, err.what()));
		}
	}

	sol::environment m_environment;
	sol::table m_scriptTable;
	ResourceModule::AssetID m_loadedScriptID;
	bool m_started{false};
};

class ECSluaScriptsSystem {
	ECSluaScriptsSystem() = default;
	ECSluaScriptsSystem(const ECSluaScriptsSystem&) = delete;
	ECSluaScriptsSystem& operator=(const ECSluaScriptsSystem&) = delete;

	ScriptModule::LuaScriptModule* m_scriptModule{nullptr};
	ResourceModule::ResourceManager* m_resourceManager{nullptr};

public:
	static ECSluaScriptsSystem& getInstance() {
		static ECSluaScriptsSystem system;
		return system;
	}

	void registerSystem(flecs::world& world, ScriptModule::LuaScriptModule* scriptModule,
	                    ResourceModule::ResourceManager* resourceManager) {
		m_scriptModule = scriptModule;
		m_resourceManager = resourceManager;
		world.system<ScriptComponent>()
		    .kind(flecs::OnUpdate)
		    .each([this](flecs::iter& it, size_t index, ScriptComponent& script) {
			    if (!m_scriptModule || !m_resourceManager)
				    return;
			    script.scriptModule = m_scriptModule;
			    script.resourceManager = m_resourceManager;
			    script.update(it.entity(index), static_cast<float>(it.delta_time()));
		    });

		world.observer<ScriptComponent>()
		    .event(flecs::OnRemove)
		    .each([](flecs::entity, ScriptComponent& script) {
			    script.end();
		    });
	}

	void startSystem(flecs::world& world) 
	{
		auto query = world.query<ScriptComponent>();
		query.each([this](const flecs::entity& entity, ScriptComponent& script) 
		{
				script.scriptModule = m_scriptModule;
				script.resourceManager = m_resourceManager;
				script.start(entity);
		});
	}

	void stopSystem(flecs::world& world) 
	{
		auto query = world.query<ScriptComponent>();
		query.each([](const flecs::entity&, ScriptComponent& script) 
		{
				script.end();
		});
	}
};
