#include "UIRenderPass.h"

#include "RenderModule.h"
#include "RenderConfig.h"

#include <vector>

namespace RenderModule::UIRenderPass
{
void render()
{
    // Optional UI toggle from RenderConfig.
    if (!RenderConfig::getInstance().getUIEnabled())
    {
        return;
    }

    auto* renderModule = RenderModule::RenderModule::GetInstance();
    if (!renderModule)
    {
        return;
    }

    auto backend = renderModule->getUIRenderBackend();
    if (!backend)
    {
        return;
    }

    const std::vector<RenderModule::RenderModule::UICallback>& callbacks = renderModule->getUICallbacks();
    if (callbacks.empty())
    {
        return;
    }

    // UI is rendered to the default framebuffer as an overlay after the main scene.
    backend->beginFrame();
    for (const auto& cb : callbacks)
    {
        if (cb)
        {
            cb(backend.get());
        }
    }
    backend->endFrame();
    backend->render();
}
} // namespace RenderModule::UIRenderPass


