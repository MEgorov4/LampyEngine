#include "LuaScriptModule.h"
#include "../LoggerModule/Logger.h"
#include "../InputModule/InputModule.h"
#include "../ObjectCoreModule/ECS/ECSModule.h"
#include "../AudioModule/AudioModule.h"

void LuaScriptModule::startup()
{
	LOG_INFO("LuaScriptModule: Startup");
	m_luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::os, sol::lib::string);

	registerLogger();

	registerEvent();
	registerInputModuleEvents();

	registerAudioModule();

	registerMathTypes();

	registerECSModule();
}

void LuaScriptModule::processCommand(const std::string& command)
{
	try {
		sol::load_result loadedScript = m_luaState.load(command);
		if (!loadedScript.valid()) {
			sol::error loadError = loadedScript;
			LOG_ERROR("Lua load error: " + std::string(loadError.what()));
			return;
		}

		sol::protected_function script = loadedScript;
		sol::protected_function_result result = script();

		if (!result.valid()) {
			sol::error execError = result;
			LOG_ERROR("Lua execution error: " + std::string(execError.what()));
		}
	}
	catch (const sol::error& e) {
		LOG_ERROR("Lua exception caught: " + std::string(e.what()));
	}
}

void LuaScriptModule::registerLogger()
{
	m_luaState.set_function("LogInfo", [](const std::string& message) {
		LOG_INFO(message);
		});
	m_luaState.set_function("LogDebug", [](const std::string& message) {
		LOG_DEBUG(message);
		});
	m_luaState.set_function("LogVerbose", [](const std::string& message) {
		LOG_VERBOSE(message);
		});
	m_luaState.set_function("LogWarning", [](const std::string& message) {
		LOG_WARNING(message);
		});
	m_luaState.set_function("LogError", [](const std::string& message) {
		LOG_ERROR(message);
		});
	m_luaState.set_function("LogFatal", [](const std::string& message) {
		LOG_FATAL(message);
		});
}

void LuaScriptModule::registerEvent()
{
	m_luaState.new_usertype<Event<int, int, int, int>>("EventInt",
		sol::constructors<Event<int, int, int, int>()>(),

		"subscribe", [](Event<int, int, int, int>& self, sol::function luaHandler) {
			return self.subscribe([luaHandler](int arg1, int arg2, int arg3, int arg4) {
				luaHandler(arg1, arg2, arg3, arg4);
				});
		},

		"unsubscribe", &Event<int, int, int, int>::unsubscribe,

		"invoke", &Event<int, int, int, int>::operator()
	);

	m_luaState.new_usertype<Event<double, double>>("EventDouble",
		sol::constructors<Event<double, double>()>(),

		"subscribe", [](Event<double, double>& self, sol::function luaHandler) {
			return self.subscribe([luaHandler](double arg1, double arg2) {
				luaHandler(arg1, arg2);
				});
		},

		"unsubscribe", &Event<double, double>::unsubscribe,

		"invoke", &Event<double, double>::operator()
	);
}

void LuaScriptModule::registerInputModuleEvents()
{
	m_luaState["OnKeyAction"] = &InputModule::getInstance().OnKeyAction;

	m_luaState["OnMousePosAction"] = &InputModule::getInstance().OnMousePosAction;

	m_luaState["OnScrollAction"] = &InputModule::getInstance().OnScrollAction;
}

void LuaScriptModule::registerAudioModule()
{
	m_luaState.set_function("PlaySoundAsync", []() {
		AudioModule::getInstance().playSoundAsync();
		});
}

void LuaScriptModule::registerMathTypes()
{
	m_luaState.new_usertype<glm::vec2>("Vec2",
		sol::constructors<glm::vec2(), glm::vec2(float, float)>(),
		"x", &glm::vec2::x,
		"y", &glm::vec2::y,

		"length", [](const glm::vec2& v) -> float {
			return glm::length(v);
		},

		"normalize", [](glm::vec2& v) {
			v = glm::normalize(v);
		},

		"__add", [](const glm::vec2& v1, const glm::vec2& v2) -> glm::vec2 {
			return v1 + v2;
		},

		"__sub", [](const glm::vec2& v1, const glm::vec2& v2) -> glm::vec2 {
			return v1 - v2;
		},

		"__mul", [](const glm::vec2& v, float scalar) -> glm::vec2 {
			return v * scalar;
		},

		"__div", [](const glm::vec2& v, float scalar) -> glm::vec2 {
			return v / scalar;
		},

		"__tostring", [](const glm::vec2& v) -> std::string {
			return std::format("Vec2({}, {})", v.x, v.y);
		},

		"projectOnto", [](const glm::vec2& v1, const glm::vec2& v2) -> glm::vec2 {
			return glm::dot(v1, glm::normalize(v2)) * glm::normalize(v2);
		},

		"perpendicular", [](const glm::vec2& v) -> glm::vec2 {
			return glm::vec2(-v.y, v.x);
		},

		"getNormalized", [](const glm::vec2& v) -> glm::vec2 {
			return glm::normalize(v);
		}
	);

	m_luaState.set_function("DotProduct2", [](const glm::vec2& v1, const glm::vec2& v2) -> float {
		return glm::dot(v1, v2);
		});

	m_luaState.set_function("Distance2", [](const glm::vec2& v1, const glm::vec2& v2) -> float {
		return glm::distance(v1, v2);
		});

	m_luaState.set_function("Lerp2", [](const glm::vec2& v1, const glm::vec2& v2, float t) -> glm::vec2 {
		return glm::mix(v1, v2, t);
		});

	m_luaState.set_function("Reflect2", [](const glm::vec2& v, const glm::vec2& normal) -> glm::vec2 {
		return glm::reflect(v, glm::normalize(normal));
		});

	m_luaState.set_function("Rotate2", [](const glm::vec2& v, float angle) -> glm::vec2 {
		float cosA = glm::cos(angle);
		float sinA = glm::sin(angle);
		return glm::vec2(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
		});

	m_luaState.new_usertype<glm::vec3>("Vec3",
		sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
		"x", &glm::vec3::x,
		"y", &glm::vec3::y,
		"z", &glm::vec3::z,

		"length", [](const glm::vec3& v) -> float {
			return glm::length(v);
		},

		"normalize", [](glm::vec3& v) {
			v = glm::normalize(v);
		},

		"getNormalized", [](const glm::vec3& v) -> glm::vec3 {
			return glm::normalize(v);
		},

		"__add", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 {
			return v1 + v2;
		},

		"__sub", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 {
			return v1 - v2;
		},

		"__mul", [](const glm::vec3& v, float scalar) -> glm::vec3 {
			return v * scalar;
		},

		"__div", [](const glm::vec3& v, float scalar) -> glm::vec3 {
			return v / scalar;
		},

		"__tostring", [](const glm::vec3& v) -> std::string {
			return std::format("Vec2({}, {}, {})", v.x, v.y, v.z);
		},

		"dot", [](const glm::vec3& v1, const glm::vec3& v2) -> float {
			return glm::dot(v1, v2);
		},

		"cross", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 {
			return glm::cross(v1, v2);
		},

		"projectOnto", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 {
			return glm::dot(v1, glm::normalize(v2)) * glm::normalize(v2);
		},

		"reflect", [](const glm::vec3& v, const glm::vec3& normal) -> glm::vec3 {
			return glm::reflect(v, glm::normalize(normal));
		}
	);

	m_luaState.set_function("DotProduct3", [](const glm::vec3& v1, const glm::vec3& v2) -> float {
		return glm::dot(v1, v2);
		});

	m_luaState.set_function("CrossProduct3", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 {
		return glm::cross(v1, v2);
		});

	m_luaState.set_function("Distance3", [](const glm::vec3& v1, const glm::vec3& v2) -> float {
		return glm::distance(v1, v2);
		});

	m_luaState.set_function("Lerp3", [](const glm::vec3& v1, const glm::vec3& v2, float t) -> glm::vec3 {
		return glm::mix(v1, v2, t);
		});

	m_luaState.set_function("Reflect3", [](const glm::vec3& v, const glm::vec3& normal) -> glm::vec3 {
		return glm::reflect(v, glm::normalize(normal));
		});
}

void LuaScriptModule::registerECSModule()
{
	m_luaState.new_usertype<flecs::world>("World",
		"entity", [](flecs::world& w, const std::string& name) -> flecs::entity {
			auto entity = w.entity(name.c_str());
			entity.set<PositionComponent>({});
			return entity;
		},
		"find", [](flecs::world& w, const std::string& name) -> flecs::entity {
			return w.lookup(name.c_str());
		}
	);

	m_luaState.new_usertype<PositionComponent>("PositionComponent",
		"x", &PositionComponent::x,
		"y", &PositionComponent::y,
		"z", &PositionComponent::z);

	m_luaState.new_usertype<flecs::entity>("Entity",
		"id", &flecs::entity::id,
		"destruct", &flecs::entity::destruct,
		"is_valid", &flecs::entity::is_valid,

		"get_name", [](flecs::entity& e) -> const char* {
			return e.name().c_str();
		},

		"get_position", [this](flecs::entity& e) -> glm::vec3 {
			if (auto* position = e.get<PositionComponent>()) {
				return position->toGLMVec();
			}
			return glm::vec3();
		},

		"set_position", [](flecs::entity& e, glm::vec3 pos) {
			if (e.has<PositionComponent>())
			{
				e.set<PositionComponent>({pos.x, pos.y, pos.z});
			}
		}
	);

	m_luaState.set_function("GetWorld", []() -> flecs::world& { return ECSModule::getInstance().getCurrentWorld(); });
}
