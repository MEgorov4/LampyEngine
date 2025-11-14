#pragma once

#include "ScriptVM.h"

#include <sol/table.hpp>

#include <string>
#include <vector>

namespace ScriptModule
{
class ScriptExecutor
{
  public:
    explicit ScriptExecutor(std::string name);

    void attachVM(ScriptVM* vm);

    void addScript(sol::table scriptTable);
    void removeScript(const sol::table& scriptTable);

    void executeStart();
    void executeUpdate(float deltaSeconds);

    size_t activeScriptCount() const noexcept { return m_scripts.size(); }

  private:
    struct TrackedScript
    {
        sol::table table;
        bool started{false};
    };

    ScriptVM* m_vm{nullptr};
    std::string m_name;
    std::vector<TrackedScript> m_scripts;
};
} // namespace ScriptModule

