#include "LuaScriptModule.h"
#include "../LoggerModule/Logger.h"
#include "../InputModule/InputModule.h"
#include "../ObjectCoreModule/ECS/ECSLuaScriptsSystem.h"
#include "../ObjectCoreModule/ECS/ECSModule.h"
void LuaScriptModule::startup()
{
	LOG_INFO("LuaScriptModule: Startup");
	m_luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::os, sol::lib::string);
	registerLogger();
	registerEvent();
	registerInputModuleEvents();
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

	m_luaState.new_usertype<Event<double, double>>("EventInt",
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
