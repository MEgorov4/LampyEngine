#pragma once

#include <EngineMinimal.h>
#include "RenderEntityTracker.h"
#include "Abstract/RenderObject.h"
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <glm/glm.hpp>

namespace RenderModule
{
class RenderObjectFactory
{
public:
    static RenderObject createFromState(EntityRenderState& state);
    
    static void updateTransform(RenderObject& obj, const EntityRenderState& state);
    
    static void updateResources(RenderObject& obj, const EntityRenderState& state);
    
private:
    static glm::mat4 computeModelMatrix(const PositionComponent& pos, 
                                        const RotationComponent& rot, 
                                        const ScaleComponent& scale);
};

} // namespace RenderModule

