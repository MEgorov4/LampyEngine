#pragma once

#include <EngineMinimal.h>
#include "RenderEntityTracker.h"
#include "RenderListManager.h"
#include "RenderObjectFactory.h"
#include <Modules/ObjectCoreModule/ECS/Events.h>

namespace RenderModule
{
class TransformUpdater
{
private:
    RenderEntityTracker& m_tracker;
    RenderListManager& m_listManager;
    
public:
    TransformUpdater(RenderEntityTracker& tracker, RenderListManager& listManager);
    
    void updateFromEvent(const Events::ECS::RenderFrameData& frameData);
    
private:
    bool updateObjectTransform(const Events::ECS::ObjectTransformData& transform);
};

} // namespace RenderModule

