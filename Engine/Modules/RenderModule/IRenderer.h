#pragma once

#include <EngineMinimal.h>
#include "RenderGraph/RenderGraph.h"
#include "Abstract/ITexture.h"

namespace ECSModule { class ECSModule; }

namespace RenderModule {
class IRenderer {
    TextureHandle m_activeTextureHandle{};
    Event<>::Subscription m_updEcsSub{};
    ECSModule::ECSModule* m_ecsModule{};
    RenderGraph m_renderGraph;
public:
    IRenderer();
    virtual ~IRenderer();

    void render();
    virtual void waitIdle() = 0;

    void updateRenderList();    // больше не const: будет триггерить перепарс сцены/данных
    void postInit();

    TextureHandle getOutputRenderHandle(int w, int h);
};

} // namespace RenderModule