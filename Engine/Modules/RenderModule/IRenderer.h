#pragma once

#include <EngineMinimal.h>
#include "RenderGraph/RenderGraph.h"
#include "Abstract/ITexture.h"
#include <EngineContext/Core/ModuleEventBinder.h>
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
    ECSModule::ECSModule* m_ecsModule{};  // Остается для случаев прямого доступа, но не используется для событий
    RenderGraph m_renderGraph;
    EngineCore::Base::ModuleEventBinder m_eventBinder;  // EventBus для подписки на события ECS
    
    // Система отслеживания изменений и diff
    RenderEntityTracker m_entityTracker;  // Трекер состояния сущностей
    RenderSystemObserver m_renderObserver;  // Observer для отслеживания изменений в ECS
    
    // Управление рендер-листом и синхронизацией
    RenderListManager m_listManager;  // Менеджер рендер-листа
    std::unique_ptr<CameraUpdater> m_cameraUpdater;  // Обновление камеры
    TransformUpdater m_transformUpdater;  // Обновление трансформаций
    
    bool m_needsFullRebuild = true;  // Флаг для полной перестройки рендер-листа (при первом запуске)
    
public:
    IRenderer();
    virtual ~IRenderer();

    void render();
    virtual void waitIdle() = 0;

    void updateRenderList();    // Применяет diff или делает полную перестройку
    void applyRenderDiff(const RenderDiff& diff);  // Применяет diff изменений к рендер-листу
    void rebuildRenderList();   // Полная перестройка рендер-листа (для первого запуска)
    void postInit();
    void setupEventSubscriptions();  // Настраивает подписки на события ECS
    void setupRenderObservers();    // Инициализирует observers для отслеживания изменений в ECS
    void drawWorldGrid();  // Рисует сетку мира

    TextureHandle getOutputRenderHandle(int w, int h);

    // Методы для добавления отладочных примитивов
    void drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color = glm::vec3(1.0f, 0.0f, 0.0f));
    void drawBox(const glm::vec3& center, const glm::vec3& size, const glm::vec3& color = glm::vec3(0.0f, 1.0f, 0.0f));
    void drawSphere(const glm::vec3& center, float radius, const glm::vec3& color = glm::vec3(0.0f, 0.0f, 1.0f));
    
private:
    // Обработчики событий от ECS
    void onWorldOpened(const Events::ECS::WorldOpened& event);
    void onWorldClosed(const Events::ECS::WorldClosed& event);
    void onEntityCreated(const Events::ECS::EntityCreated& event);
    void onEntityDestroyed(const Events::ECS::EntityDestroyed& event);
    void onComponentChanged(const Events::ECS::ComponentChanged& event);
    void onRenderFrameData(const Events::ECS::RenderFrameData& event);  // Данные для рендеринга кадра от ECSModule
    void updateLightsFromECS();  // Обновляет источники света из ECS
};

} // namespace RenderModule