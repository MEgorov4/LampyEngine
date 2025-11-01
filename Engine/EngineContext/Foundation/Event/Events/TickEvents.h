#pragma once

namespace EngineCore::Foundation::EventContracts
{
    /// Regular tick event for minor subsystems (input, physics, scripting)
    struct TickMinor
    {
        float deltaTime;
    };

    /// Tick event for major systems (render, ecs, etc.)
    struct TickMajor
    {
        float deltaTime;
    };

    /// Render pass cycle tick (useful for post-frame tasks)
    struct TickRender
    {
        float deltaTime;
    };
}