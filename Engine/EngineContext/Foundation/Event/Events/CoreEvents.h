#pragma once
#include <string>

namespace EngineCore::Foundation::EventContracts
{
    /// Fired once the engine systems have been constructed but before modules start.
    struct EnginePreInit {};

    /// Fired after all major modules are initialized.
    struct EngineInitialized {};

    /// Fired before the engine begins its main loop.
    struct EngineStart {};

    /// Fired once per frame before ticking modules.
    struct EnginePreTick { float deltaTime; };

    /// Fired once per frame after ticking modules.
    struct EnginePostTick { float deltaTime; };

    /// Fired before shutdown.
    struct EngineShutdown {};
}
