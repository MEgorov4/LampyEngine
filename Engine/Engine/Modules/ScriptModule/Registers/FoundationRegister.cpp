#include "FoundationRegister.h"

#include "FoundationRegister.h"

#include <Foundation/Log/LogVerbosity.h>
#include <Foundation/Log/LoggerMacro.h>

namespace ScriptModule
{
namespace
{
constexpr std::string_view kScriptLogCategory = "Script";
}

void FoundationRegister::registerTypes(sol::state&, sol::environment& env)
{
    env.set_function("LogInfo", [](const std::string& msg) { LT_LOG(LogVerbosity::Info, kScriptLogCategory.data(), msg); });
    env.set_function("LogDebug",
                     [](const std::string& msg) { LT_LOG(LogVerbosity::Debug, kScriptLogCategory.data(), msg); });
    env.set_function("LogVerbose", [](const std::string& msg)
                    { LT_LOG(LogVerbosity::Verbose, kScriptLogCategory.data(), msg); });
    env.set_function("LogWarning", [](const std::string& msg)
                    { LT_LOG(LogVerbosity::Warning, kScriptLogCategory.data(), msg); });
    env.set_function("LogError", [](const std::string& msg) { LT_LOG(LogVerbosity::Error, kScriptLogCategory.data(), msg); });
    env.set_function("LogFatal", [](const std::string& msg) { LT_LOG(LogVerbosity::Fatal, kScriptLogCategory.data(), msg); });
}
} // namespace ScriptModule

