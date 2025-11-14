#pragma once

#include "../../IScriptRegister.h"

namespace ScriptModule
{
class RenderComponentsRegister final : public IScriptRegister
{
  public:
    void registerTypes(sol::state& state, sol::environment& env) override;
};
} // namespace ScriptModule

