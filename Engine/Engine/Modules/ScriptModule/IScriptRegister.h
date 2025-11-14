#pragma once

#include <sol/sol.hpp>

namespace ScriptModule
{
class IScriptRegister
{
  public:
    virtual ~IScriptRegister() = default;
    virtual void registerTypes(sol::state& state, sol::environment& env) = 0;
};
} // namespace ScriptModule

