#pragma once

#include <EngineMinimal.h>

namespace UIModule
{
class NuklearBackend;

/// <summary>
/// Simple UI system that builds a minimal HUD using Nuklear.
/// Invoked from UIModule via the UI render callback.
/// </summary>
class UISystem
{
    NuklearBackend* m_backend = nullptr;

  public:
    explicit UISystem(NuklearBackend* backend) : m_backend(backend) {}

    void update(float dt);
};
} // namespace UIModule


