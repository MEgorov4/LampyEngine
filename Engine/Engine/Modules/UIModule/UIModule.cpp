#include "UIModule.h"

#include <Modules/InputModule/InputModule.h>
#include <Modules/WindowModule/WindowModule.h>
#include <Modules/WindowModule/Window.h>
#include <Modules/RenderModule/RenderModule.h>

#include "NuklearBackend.h"
#include "UISystem.h"

namespace UIModule
{
UIModule::~UIModule() = default;

void UIModule::startup()
{
    LT_LOGI("UIModule", "Startup");

    m_inputModule  = GCM(InputModule::InputModule);
    m_windowModule = GCM(WindowModule::WindowModule);
    m_renderModule = GCM(RenderModule::RenderModule);

    if (!m_inputModule || !m_windowModule || !m_renderModule)
    {
        LT_LOGE("UIModule", "Failed to acquire required modules");
        return;
    }

    m_inputSub = m_inputModule->OnEvent.subscribe(std::bind(&UIModule::onEvent, this, std::placeholders::_1));

    // Create Nuklear backend and register it with RenderModule.
    m_backend  = std::make_shared<NuklearBackend>(m_windowModule);
    m_uiSystem = std::make_unique<UISystem>(static_cast<NuklearBackend*>(m_backend.get()));
    m_renderModule->setUIRenderBackend(m_backend);
    m_renderModule->addUICallback([this](RenderModule::IUIRenderBackend* backend) { onUIFrame(backend); });
}

void UIModule::shutdown()
{
    LT_LOGI("UIModule", "Shutdown");

    m_inputSub = {};
    m_uiSystem.reset();
    m_backend.reset();
}

void UIModule::onEvent(const SDL_Event& event)
{
    if (m_backend)
    {
        m_backend->processSDLEvent(event);
    }
}

void UIModule::onUIFrame(RenderModule::IUIRenderBackend* backend)
{
    if (!backend || !m_uiSystem)
        return;

    // Backend is guaranteed to be NuklearBackend in the current implementation.
    m_uiSystem->update(0.0f);
}
} // namespace UIModule


