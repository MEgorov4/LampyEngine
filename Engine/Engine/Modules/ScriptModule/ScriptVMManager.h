#pragma once

#include "ScriptVM.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace ScriptModule
{
class ScriptVMManager
{
  public:
    ScriptVMManager() = default;

    void initializeAll(const ScriptVMProfileMap& profiles);
    void shutdownAll();

    ScriptVM* runtimeVM() noexcept { return getVM(ScriptVMType::Runtime); }
    ScriptVM* editorVM() noexcept { return getVM(ScriptVMType::Editor); }
    ScriptVM* devVM() noexcept { return getVM(ScriptVMType::Dev); }
    ScriptVM* clientVM() noexcept { return getVM(ScriptVMType::Client); }
    ScriptVM* serverVM() noexcept { return getVM(ScriptVMType::Server); }
    ScriptVM* toolsVM() noexcept { return getVM(ScriptVMType::Tools); }

    ScriptVM* getVM(ScriptVMType type) noexcept;
    const ScriptVM* getVM(ScriptVMType type) const noexcept;

    ScriptVM& addCustomVM(std::string name, const ScriptVMProfile& profile);
    ScriptVM* findCustomVM(std::string_view name) noexcept;

  private:
    ScriptVM& createVM(ScriptVMType type, const ScriptVMProfile& profile);

  private:
    std::unordered_map<ScriptVMType, std::unique_ptr<ScriptVM>> m_vms;
    std::unordered_map<std::string, std::unique_ptr<ScriptVM>> m_customVMs;
};
} // namespace ScriptModule

