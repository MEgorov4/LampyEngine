#pragma once 


#include <sol/sol.hpp>
#include <sol/forward.hpp>
#include <sol/reference.hpp>
#include <vector>

class LuaScriptExecutor
{
	std::vector<sol::table> m_tables;

public:
	void addScript(const sol::table& table);
	void removeScript(const sol::table& table);
	void executeStart();
	void executeUpdate(float deltaSeconds);
};