#pragma once

#include <EngineMinimal.h>
#include "RenderGraph/RenderGraph.h"
#include "Abstract/ITexture.h"
#include <Core/ModuleEventBinder.h>
#include <Modules/ObjectCoreModule/ECS/Events.h>
#include "RenderEntityTracker.h"
#include "RenderSystemObserver.h"
#include "RenderListManager.h"
#include "CameraUpdater.h"
#include "TransformUpdater.h"

namespace ECSModule { class ECSModule; }

namespace RenderModule {
class IRenderer {
    TextureHandle m_activeTextureHandle{};
    Event<>::Subscription m_updEcsSub{};
    ECSModule::ECSModule* m_ecsModule{};
    RenderGraph m_renderGraph;
    EngineCore::Base::ModuleEventBinder m_eventBinder;
    
    RenderEntityTracker m_entityTracker;
    RenderSystemObserver m_renderObserver;
    
    RenderListManager m_listManager;
    std::unique_ptr<CameraUpdater> m_cameraUpdater;
    TransformUpdater m_transformUpdater;
    
    bool m_needsFullRebuild = true;
    
public:
    IRenderer();
    virtual ~IRenderer();

    void render();
    virtual void waitIdle() = 0;

    void updateRenderList();
    void applyRenderDiff(const RenderDiff& diff);
    void rebuildRenderList();
    void postInit();
    void setupEventSubscriptions();
    void setupRenderObservers();
    void drawWorldGrid();

    TextureHandle getOutputRenderHandle(int w, int h);

    void drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color = glm::vec3(1.0f, 0.0f, 0.0f));
    void drawBox(const glm::vec3& center, const glm::vec3& size, const glm::vec3& color = glm::vec3(0.0f, 1.0f, 0.0f));
    void drawSphere(const glm::vec3& center, float radius, const glm::vec3& color = glm::vec3(0.0f, 0.0f, 1.0f));
    
private:
    void onWorldOpened(const Events::ECS::WorldOpened& event);
    void onWorldClosed(const Events::ECS::WorldClosed& event);
    void onEntityCreated(const Events::ECS::EntityCreated& event);
    void onEntityDestroyed(const Events::ECS::EntityDestroyed& event);
    void onComponentChanged(const Events::ECS::ComponentChanged& event);
    void onRenderFrameData(const Events::ECS::RenderFrameData& event);
    void updateLightsFromECS();
};

} // namespace RenderModule