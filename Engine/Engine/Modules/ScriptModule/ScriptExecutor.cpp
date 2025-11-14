#include "ScriptExecutor.h"

#include <Foundation/Log/LoggerMacro.h>

#include <algorithm>
#include <format>

namespace ScriptModule
{
namespace
{
constexpr std::string_view kExecutorCategory = "ScriptExecutor";
}

ScriptExecutor::ScriptExecutor(std::string name) : m_name(std::move(name)) {}

void ScriptExecutor::attachVM(ScriptVM* vm)
{
    m_vm = vm;
}

void ScriptExecutor::addScript(sol::table scriptTable)
{
    if (!scriptTable.valid())
        return;

    auto it = std::find_if(
        m_scripts.begin(), m_scripts.end(), [&scriptTable](const TrackedScript& tracked)
        { return tracked.table == scriptTable; });
    if (it != m_scripts.end())
        return;

    m_scripts.push_back({scriptTable, false});
}

void ScriptExecutor::removeScript(const sol::table& scriptTable)
{
    m_scripts.erase(std::remove_if(m_scripts.begin(),
                                   m_scripts.end(),
                                   [&scriptTable](const TrackedScript& tracked)
                                   { return tracked.table == scriptTable; }),
                    m_scripts.end());
}

void ScriptExecutor::executeStart()
{
    for (auto& tracked : m_scripts)
    {
        if (tracked.started || !tracked.table.valid())
            continue;

        sol::object potential = tracked.table["Start"];
        if (!potential.valid() || potential.get_type() != sol::type::function)
        {
            tracked.started = true;
            continue;
        }

        sol::protected_function func = potential;
        sol::protected_function_result result = func();
        if (!result.valid())
        {
            sol::error err = result;
            LT_LOGE(kExecutorCategory.data(), std::format("[{}] Start() error: {}", m_name, err.what()));
        }
        else
        {
            LT_LOG(::EngineCore::Foundation::LogVerbosity::Debug,
                   kExecutorCategory.data(),
                   std::format("[{}] Start() executed", m_name));
        }
        tracked.started = true;
    }
}

void ScriptExecutor::executeUpdate(float deltaSeconds)
{
    for (auto it = m_scripts.begin(); it != m_scripts.end();)
    {
        if (!it->table.valid())
        {
            it = m_scripts.erase(it);
            continue;
        }

        sol::object potential = it->table["Update"];
        if (!potential.valid() || potential.get_type() != sol::type::function)
        {
            ++it;
            continue;
        }

        sol::protected_function func = potential;
        sol::protected_function_result result = func(deltaSeconds);
        if (!result.valid())
        {
            sol::error err = result;
            LT_LOGE(kExecutorCategory.data(), std::format("[{}] Update() error: {}", m_name, err.what()));
        }
        ++it;
    }
}
} // namespace ScriptModule

