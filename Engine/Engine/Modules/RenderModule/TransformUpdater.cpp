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
    auto* state = m_tracker.getState(transform.entityId);
    if (!state)
    {
        return false;
    }
    
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
    
    size_t* pIndex = m_listManager.getObjectIndex(transform.entityId);
    if (!pIndex)
    {
        return false;
    }
    
    size_t objIndex = *pIndex;
    if (!m_listManager.isValidIndex(objIndex))
    {
        m_listManager.removeObject(transform.entityId);
        return false;
    }
    
    auto& objects = m_listManager.getObjects();
    RenderObjectFactory::updateTransform(objects[objIndex], *state);
    
    return true;
}

} // namespace RenderModule

