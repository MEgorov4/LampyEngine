#pragma once

#include <EngineMinimal.h>

#include "UISystem.h"

#include <memory>

namespace InputModule
{
class InputModule;
}

namespace WindowModule
{
class WindowModule;
}

namespace RenderModule
{
class RenderModule;
class IUIRenderBackend;
}

union SDL_Event;

namespace UIModule
{

/// <summary>
/// High-level UI module which wires input and render callbacks to a concrete UI backend (e.g., Nuklear).
/// </summary>
class UIModule : public IModule
{
    InputModule::InputModule* m_inputModule   = nullptr;
    WindowModule::WindowModule* m_windowModule = nullptr;
    RenderModule::RenderModule* m_renderModule = nullptr;

    std::shared_ptr<RenderModule::IUIRenderBackend> m_backend;
    Event<SDL_Event>::Subscription m_inputSub;

    std::unique_ptr<UISystem> m_uiSystem;

  public:
    ~UIModule() override;
    void startup() override;
    void shutdown() override;
  private:
    void onEvent(const SDL_Event& event);
    void onUIFrame(RenderModule::IUIRenderBackend* backend);
};
} // namespace UIModule


