#include "TransformUpdater.h"
#include "RenderObjectFactory.h"
#include <EngineMinimal.h>

namespace RenderModule
{

TransformUpdater::TransformUpdater(RenderEntityTracker& tracker, RenderListManager& listManager)
    : m_tracker(tracker)
    , m_listManager(listManager)
{
}

void TransformUpdater::updateFromEvent(const Events::ECS::RenderFrameData& frameData)
{
    ZoneScopedN("TransformUpdater::updateFromEvent");
    
    for (const auto& transform : frameData.objectsTransforms)
    {
        updateObjectTransform(transform);
    }
}

bool TransformUpdater::updateObjectTransform(const Events::ECS::ObjectTransformData& transform)
{
    // Проверяем, существует ли сущность в трекере (может быть удалена)
    auto* state = m_tracker.getState(transform.entityId);
    if (!state)
    {
        // Сущность не найдена в трекере - возможно была удалена, но еще есть в RenderFrameData
        // Это нормальная ситуация при удалении, пропускаем
        return false;
    }
    
    // Обновляем состояние трансформации в трекере
    state->position.x = transform.posX;
    state->position.y = transform.posY;
    state->position.z = transform.posZ;
    
    state->rotation.x = transform.rotX;
    state->rotation.y = transform.rotY;
    state->rotation.z = transform.rotZ;
    state->rotation.qx = transform.rotQX;
    state->rotation.qy = transform.rotQY;
    state->rotation.qz = transform.rotQZ;
    state->rotation.qw = transform.rotQW;
    
    state->scale.x = transform.scaleX;
    state->scale.y = transform.scaleY;
    state->scale.z = transform.scaleZ;
    
    // Находим индекс объекта по entityId
    size_t* pIndex = m_listManager.getObjectIndex(transform.entityId);
    if (!pIndex)
    {
        // Объекта нет в маппинге - возможно он был добавлен через diff, но diff еще не применен
        // Или объект был удален, но diff еще не применен
        // В любом случае, если есть в трекере, но нет в маппинге - это проблема синхронизации
        // Пропускаем до следующего кадра, когда diff будет применен
        return false;
    }
    
    size_t objIndex = *pIndex;
    if (!m_listManager.isValidIndex(objIndex))
    {
        // Индекс вышел за границы - проблема синхронизации
        // Удаляем из маппинга и продолжаем
        m_listManager.removeObject(transform.entityId);
        return false;
    }
    
    // Обновляем матрицу модели в объекте
    auto& objects = m_listManager.getObjects();
    RenderObjectFactory::updateTransform(objects[objIndex], *state);
    
    return true;
}

} // namespace RenderModule

