#pragma once
#include <string>

namespace EngineCore::Foundation::EventContracts
{
    /// A specific module has been started up.
    struct ModuleStartup
    {
        std::string moduleName;
    };

    /// A specific module has been shut down.
    struct ModuleShutdown
    {
        std::string moduleName;
    };

    /// Request module reload (e.g. hot-reload in editor)
    struct ModuleReload
    {
        std::string moduleName;
    };
}
