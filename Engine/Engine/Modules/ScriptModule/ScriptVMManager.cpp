#include "ScriptVMManager.h"

#include <Foundation/Log/LoggerMacro.h>

#include <format>

namespace ScriptModule
{
namespace
{
constexpr std::string_view kVMManagerLogCategory = "ScriptVMManager";

std::string_view toString(ScriptVMType type)
{
    switch (type)
    {
    case ScriptVMType::Runtime:
        return "Runtime";
    case ScriptVMType::Editor:
        return "Editor";
    case ScriptVMType::Dev:
        return "Dev";
    case ScriptVMType::Client:
        return "Client";
    case ScriptVMType::Server:
        return "Server";
    case ScriptVMType::Tools:
        return "Tools";
    default:
        return "Custom";
    }
}
} // namespace

void ScriptVMManager::initializeAll(const ScriptVMProfileMap& profiles)
{
    shutdownAll();
    for (const auto& [type, profile] : profiles)
    {
        createVM(type, profile);
    }
}

void ScriptVMManager::shutdownAll()
{
    for (auto& [_, vm] : m_vms)
    {
        if (vm)
            vm->shutdown();
    }
    for (auto& [_, vm] : m_customVMs)
    {
        if (vm)
            vm->shutdown();
    }
    m_vms.clear();
    m_customVMs.clear();
}

ScriptVM* ScriptVMManager::getVM(ScriptVMType type) noexcept
{
    auto it = m_vms.find(type);
    if (it == m_vms.end())
        return nullptr;
    return it->second.get();
}

const ScriptVM* ScriptVMManager::getVM(ScriptVMType type) const noexcept
{
    auto it = m_vms.find(type);
    if (it == m_vms.end())
        return nullptr;
    return it->second.get();
}

ScriptVM& ScriptVMManager::addCustomVM(std::string name, const ScriptVMProfile& profile)
{
    auto vm = std::make_unique<ScriptVM>(name);
    vm->configure(profile);
    vm->init();
    auto* raw = vm.get();
    m_customVMs.emplace(std::move(name), std::move(vm));
    LT_LOGI(kVMManagerLogCategory.data(), std::format("Custom VM [{}] initialized", raw->name()));
    return *raw;
}

ScriptVM* ScriptVMManager::findCustomVM(std::string_view name) noexcept
{
    auto it = m_customVMs.find(std::string(name));
    if (it == m_customVMs.end())
        return nullptr;
    return it->second.get();
}

ScriptVM& ScriptVMManager::createVM(ScriptVMType type, const ScriptVMProfile& profile)
{
    std::string vmName = profile.name.empty() ? std::string(toString(type)) : profile.name;
    auto vm           = std::make_unique<ScriptVM>(vmName);
    vm->configure(profile);
    vm->init();
    auto* raw = vm.get();
    m_vms[type] = std::move(vm);
    LT_LOGI(kVMManagerLogCategory.data(), std::format("{} VM initialized", vmName));
    return *raw;
}
} // namespace ScriptModule

