#pragma once

#include "../IScriptRegister.h"
#include "../ScriptServices.h"

namespace ScriptModule
{
class InputRegister final : public IScriptRegister
{
  public:
    explicit InputRegister(IInputScriptService* service) : m_service(service) {}

    void registerTypes(sol::state& state, sol::environment& env) override;

  private:
    IInputScriptService* m_service{nullptr};
};
} // namespace ScriptModule

