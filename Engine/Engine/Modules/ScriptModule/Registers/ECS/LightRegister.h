#pragma once

#include "../../IScriptRegister.h"

namespace ScriptModule
{
class LightRegister final : public IScriptRegister
{
  public:
    void registerTypes(sol::state& state, sol::environment& env) override;
};
} // namespace ScriptModule

