#pragma once

#include "IScriptRegister.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace ScriptModule
{
enum class ScriptVMType
{
    Runtime,
    Editor,
    Dev,
    Client,
    Server,
    Tools,
    Custom
};

struct SandboxRule
{
    std::string table;
    std::string member;
};

struct ScriptVMProfile
{
    std::string name;
    std::vector<sol::lib> libraries;
    std::vector<SandboxRule> restrictions;
    std::vector<std::function<std::shared_ptr<IScriptRegister>()>> registerFactories;
    bool autoExecOnLoad{false};
};

using ScriptVMProfileMap = std::unordered_map<ScriptVMType, ScriptVMProfile>;
} // namespace ScriptModule

