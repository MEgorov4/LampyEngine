#pragma once

#include "../IScriptRegister.h"

namespace ScriptModule
{
class ImGUIRegister final : public IScriptRegister
{
  public:
    void registerTypes(LuaScriptModule& module, sol::state& state) override;
};
} // namespace ScriptModule

