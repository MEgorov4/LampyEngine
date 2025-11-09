#pragma once

#include <sol/forward.hpp>

namespace ScriptModule
{
class LuaScriptModule;

class IScriptRegister
{
  public:
    virtual ~IScriptRegister() = default;
    virtual void registerTypes(LuaScriptModule& module, sol::state& state) = 0;
};
} // namespace ScriptModule

