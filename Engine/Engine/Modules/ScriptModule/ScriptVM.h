#pragma once

#include "ScriptVMProfile.h"

#include <sol/sol.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <format>

namespace ScriptModule
{
class ScriptVM
{
  public:
    explicit ScriptVM(std::string name);
    ~ScriptVM();

    void configure(const ScriptVMProfile& profile);
    bool init();
    void shutdown();

    bool isInitialized() const noexcept { return m_initialized; }

    sol::state& state() noexcept { return m_state; }
    sol::environment& environment() noexcept { return m_environment; }
    const std::string& name() const noexcept { return m_name; }

    bool runString(const std::string& source);
    bool runFile(const std::string& path);

    template <typename... Args>
    bool call(const std::string& functionName, Args&&... args)
    {
        try
        {
            sol::protected_function fn = findFunction(functionName);
            if (!fn.valid())
            {
                logError(std::string("Function not found: ") + functionName);
                return false;
            }
            sol::protected_function_result result = fn(std::forward<Args>(args)...);
            return handleResult(result, functionName);
        }
        catch (const sol::error& err)
        {
            logError(std::format("Exception in call({}): {}", functionName, err.what()));
            return false;
        }
        catch (const std::exception& ex)
        {
            logError(std::format("Exception in call({}): {}", functionName, ex.what()));
            return false;
        }
        catch (...)
        {
            logError(std::format("Unknown exception in call({})", functionName));
            return false;
        }
    }

  private:
    void openLibraries();
    void applySandbox();
    void registerApi();
    void instantiateRegisters();
    sol::protected_function findFunction(const std::string& functionName);
    bool handleResult(const sol::protected_function_result& result, std::string_view context);
    void logError(std::string_view message);

  private:
    std::string m_name;
    sol::state m_state;
    sol::environment m_environment;
    std::vector<sol::lib> m_libraries;
    std::vector<SandboxRule> m_restrictions;
    std::vector<std::function<std::shared_ptr<IScriptRegister>()>> m_registerFactories;
    std::vector<std::shared_ptr<IScriptRegister>> m_registers;
    bool m_autoExecOnLoad{false};
    bool m_initialized{false};
};
} // namespace ScriptModule

