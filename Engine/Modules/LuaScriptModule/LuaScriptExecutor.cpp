#include "LuaScriptExecutor.h"
#include "algorithm"

void LuaScriptExecutor::addScript(const sol::table& table)
{
	if (std::find(m_tables.begin(), m_tables.end(), table) == m_tables.end())
	{
		m_tables.push_back(table);
	}
}

void LuaScriptExecutor::removeScript(const sol::table& table)
{
	m_tables.erase(std::find(m_tables.begin(), m_tables.end(), table));
}

void LuaScriptExecutor::executeStart()
{
	for (auto& table : m_tables)
	{
		if (table.valid())
		{
			sol::function startFunction = table["Start"];
			if (startFunction.valid())
			{
				startFunction();
			}
		}
	}
}

void LuaScriptExecutor::executeUpdate(float deltaSeconds)
{
	for (auto& table : m_tables)
	{
		if (table.valid())
		{
			sol::function updateFunction = table["Update"];

			if (updateFunction.valid())
			{
				updateFunction(deltaSeconds);
			}
		}
	}
}
