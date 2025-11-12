#pragma once

#include <EngineMinimal.h>
#include "RenderContext.h"
#include <Modules/ObjectCoreModule/ECS/Events.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace RenderModule
{
class CameraUpdater
{
private:
    RenderContext* m_context;
    
public:
    explicit CameraUpdater(RenderContext* context);
    
    void updateFromEvent(const Events::ECS::CameraRenderData& cameraData);
    
private:
    float computeAspectRatio() const;
    
    void computeCameraVectors(const glm::quat& rotation, 
                               glm::vec3& forward, 
                               glm::vec3& up) const;
};

} // namespace RenderModule

