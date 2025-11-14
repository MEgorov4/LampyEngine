#pragma once

#include "../IScriptRegister.h"
#include "../ScriptServices.h"

namespace ScriptModule
{
class AudioRegister final : public IScriptRegister
{
  public:
    explicit AudioRegister(IAudioScriptService* service) : m_service(service) {}

    void registerTypes(sol::state& state, sol::environment& env) override;

  private:
    IAudioScriptService* m_service{nullptr};
};
} // namespace ScriptModule

