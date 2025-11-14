#include "ScriptVM.h"

#include <Foundation/Log/LoggerMacro.h>

#include <format>

namespace ScriptModule
{
namespace
{
constexpr std::string_view kScriptVMLogCategory = "ScriptVM";
}

ScriptVM::ScriptVM(std::string name) : m_name(std::move(name)) {}

ScriptVM::~ScriptVM()
{
    shutdown();
}

void ScriptVM::configure(const ScriptVMProfile& profile)
{
    m_libraries    = profile.libraries;
    m_registerFactories = profile.registerFactories;
    m_restrictions = profile.restrictions;
    m_autoExecOnLoad = profile.autoExecOnLoad;
    m_registers.clear();
}

bool ScriptVM::init()
{
    if (m_initialized)
        return true;

    openLibraries();
    m_environment = sol::environment(m_state, sol::create, m_state.globals());
    instantiateRegisters();
    registerApi();
    applySandbox();

    m_initialized = true;
    LT_LOGI(kScriptVMLogCategory.data(), std::format("VM [{}] initialized", m_name));
    return true;
}

void ScriptVM::shutdown()
{
    if (!m_initialized)
        return;

    m_environment = sol::environment();
    m_state = sol::state{};
    m_registers.clear();
    m_initialized = false;
    LT_LOGI(kScriptVMLogCategory.data(), std::format("VM [{}] shutdown", m_name));
}

bool ScriptVM::runString(const std::string& source)
{
    if (!m_initialized && !init())
        return false;

    sol::load_result chunk = m_state.load(source);
    if (!chunk.valid())
    {
        sol::error err = chunk;
        logError(std::format("Failed to load chunk in {}: {}", m_name, err.what()));
        return false;
    }

    sol::function func = chunk;
    sol::set_environment(m_environment, func);
    sol::protected_function protectedFunc = func;
    sol::protected_function_result result = protectedFunc();
    return handleResult(result, "runString");
}

bool ScriptVM::runFile(const std::string& path)
{
    if (!m_initialized && !init())
        return false;

    sol::load_result chunk = m_state.load_file(path);
    if (!chunk.valid())
    {
        sol::error err = chunk;
        logError(std::format("Failed to load file {} in {}: {}", path, m_name, err.what()));
        return false;
    }

    sol::function func = chunk;
    sol::set_environment(m_environment, func);
    sol::protected_function protectedFunc = func;
    sol::protected_function_result result = protectedFunc();
    return handleResult(result, path);
}

void ScriptVM::openLibraries()
{
    if (m_libraries.empty())
        return;

    for (sol::lib library : m_libraries)
    {
        m_state.open_libraries(library);
    }
}

void ScriptVM::applySandbox()
{
    for (const auto& rule : m_restrictions)
    {
        sol::object tableObj = m_environment[rule.table];
        if (!tableObj.valid())
        {
            tableObj = m_state[rule.table];
        }

        if (!tableObj.valid() || tableObj.get_type() != sol::type::table)
            continue;

        sol::table table = tableObj;
        table[rule.member] = sol::nil;
    }
}

void ScriptVM::registerApi()
{
    for (const auto& reg : m_registers)
    {
        if (!reg)
            continue;
        reg->registerTypes(m_state, m_environment);
    }
}

void ScriptVM::instantiateRegisters()
{
    if (!m_registers.empty())
        return;

    for (const auto& factory : m_registerFactories)
    {
        if (!factory)
            continue;
        m_registers.emplace_back(factory());
    }
}

sol::protected_function ScriptVM::findFunction(const std::string& functionName)
{
    sol::object target = m_environment[functionName];
    if (!target.valid())
    {
        target = m_state[functionName];
    }

    if (!target.valid() || target.get_type() != sol::type::function)
        return {};

    return target.as<sol::protected_function>();
}

bool ScriptVM::handleResult(const sol::protected_function_result& result, std::string_view context)
{
    if (result.valid())
        return true;

    sol::error err = result;
    logError(std::format("[{}] {}", context, err.what()));
    return false;
}

void ScriptVM::logError(std::string_view message)
{
    LT_LOGE(kScriptVMLogCategory.data(), std::format("VM [{}] {}", m_name, message));
}

} // namespace ScriptModule

