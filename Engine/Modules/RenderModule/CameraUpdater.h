#pragma once

#include <EngineMinimal.h>
#include "RenderContext.h"
#include <Modules/ObjectCoreModule/ECS/Events.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace RenderModule
{
/// Обновление данных камеры в сцене
class CameraUpdater
{
private:
    RenderContext* m_context;
    
public:
    explicit CameraUpdater(RenderContext* context);
    
    /// Обновить камеру из данных события RenderFrameData
    void updateFromEvent(const Events::ECS::CameraRenderData& cameraData);
    
private:
    /// Вычислить aspect ratio из viewport
    float computeAspectRatio() const;
    
    /// Вычислить векторы направления камеры из quaternion
    void computeCameraVectors(const glm::quat& rotation, 
                               glm::vec3& forward, 
                               glm::vec3& up) const;
};

} // namespace RenderModule

