#pragma once

#include <EngineMinimal.h>
#include "RenderContext.h"
#include "RenderConfig.h"
#include "IUIRenderBackend.h"

#include <optional>
#include <functional>
#include <memory>
#include <vector>

namespace RenderModule
{
class IRenderer;

struct RenderModuleConfig
{
    std::optional<RenderOutputMode> outputMode;
    std::optional<bool> debugPassEnabled;
    std::optional<bool> gridPassEnabled;
};

/// <summary>
/// Core render module. Owns the main renderer and render context, and optionally a UI render backend.
/// </summary>
class RenderModule : public IModule
{
  public:
    using UICallback = std::function<void(IUIRenderBackend*)>;

  private:
    std::unique_ptr<IRenderer> m_renderer;
    std::unique_ptr<RenderContext> m_context;
    std::optional<RenderOutputMode> m_configuredOutputMode;
    std::optional<bool> m_configuredDebugPassEnabled;
    std::optional<bool> m_configuredGridPassEnabled;

    /// <summary>
    /// Optional UI render backend (e.g., Nuklear) used by a UI render pass.
    /// </summary>
    std::shared_ptr<IUIRenderBackend> m_uiBackend;
    std::vector<UICallback> m_uiCallbacks;

  public:
    /// <summary>
    /// Returns the global RenderModule instance (set during startup).
    /// </summary>
    static RenderModule* GetInstance();

    void startup() override;
    void shutdown() override;

    IRenderer* getRenderer();

    void applyConfig(const RenderModuleConfig& config);

    /// <summary>
    /// Sets the UI render backend implementation.
    /// </summary>
    void setUIRenderBackend(std::shared_ptr<IUIRenderBackend> backend) { m_uiBackend = std::move(backend); }

    /// <summary>
    /// Registers a UI callback which will be invoked each UI frame with the active backend.
    /// </summary>
    void addUICallback(const UICallback& cb) { m_uiCallbacks.push_back(cb); }

    /// <summary>
    /// Accessor for the UI backend (used by the UI render pass).
    /// </summary>
    std::shared_ptr<IUIRenderBackend> getUIRenderBackend() const { return m_uiBackend; }

    /// <summary>
    /// Accessor for registered UI callbacks.
    /// </summary>
    const std::vector<UICallback>& getUICallbacks() const { return m_uiCallbacks; }

  private:
    static RenderModule* s_instance;
};
} // namespace RenderModule
