#pragma once

#include "../../IScriptRegister.h"
#include "../../ScriptServices.h"

namespace ScriptModule
{
class EntityRegister final : public IScriptRegister
{
  public:
    explicit EntityRegister(IECSWorldScriptService* service) : m_service(service) {}

    void registerTypes(sol::state& state, sol::environment& env) override;

  private:
    IECSWorldScriptService* m_service{nullptr};
};
} // namespace ScriptModule

