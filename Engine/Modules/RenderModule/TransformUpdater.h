#pragma once

#include <EngineMinimal.h>
#include "RenderEntityTracker.h"
#include "RenderListManager.h"
#include "RenderObjectFactory.h"
#include <Modules/ObjectCoreModule/ECS/Events.h>

namespace RenderModule
{
/// Обновление трансформаций объектов из событий ECS
class TransformUpdater
{
private:
    RenderEntityTracker& m_tracker;
    RenderListManager& m_listManager;
    
public:
    TransformUpdater(RenderEntityTracker& tracker, RenderListManager& listManager);
    
    /// Обновить трансформации объектов из события RenderFrameData
    void updateFromEvent(const Events::ECS::RenderFrameData& frameData);
    
private:
    /// Обновить трансформацию одного объекта
    bool updateObjectTransform(const Events::ECS::ObjectTransformData& transform);
};

} // namespace RenderModule

