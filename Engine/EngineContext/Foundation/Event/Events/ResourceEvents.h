#pragma once
#include <string>

namespace EngineCore::Foundation::EventContracts
{
    struct ResourceLoaded
    {
        std::string path;
        std::string type;
    };

    struct ResourceUnloaded
    {
        std::string path;
        std::string type;
    };

    struct ResourceReloadRequested
    {
        std::string path;
    };
}
