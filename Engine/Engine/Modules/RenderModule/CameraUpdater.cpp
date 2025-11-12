#include "CameraUpdater.h"

namespace RenderModule
{

CameraUpdater::CameraUpdater(RenderContext* context)
    : m_context(context)
{
}

void CameraUpdater::updateFromEvent(const Events::ECS::CameraRenderData& cameraData)
{
    ZoneScopedN("CameraUpdater::updateFromEvent");
    
    if (!m_context)
        return;
    
    auto& scene = m_context->scene();
    const float aspect = computeAspectRatio();
    
    const glm::vec3 camPos(cameraData.posX, cameraData.posY, cameraData.posZ);
    const glm::quat camRot(cameraData.rotQW, cameraData.rotQX, cameraData.rotQY, cameraData.rotQZ);
    const glm::quat normalizedRot = glm::normalize(camRot);
    
    glm::vec3 forward, up;
    computeCameraVectors(normalizedRot, forward, up);
    
    scene.camera.position = glm::vec4(camPos, 1.f);
    scene.camera.view = glm::lookAt(camPos, camPos + forward, up);
    scene.camera.projection = glm::perspective(
        glm::radians(cameraData.fov), 
        aspect, 
        cameraData.nearClip, 
        cameraData.farClip
    );
}

float CameraUpdater::computeAspectRatio() const
{
    if (!m_context)
        return 16.f / 9.f;
    
    const auto [w, h] = m_context->getViewport();
    return (h != 0) ? float(w) / float(h) : 16.f / 9.f;
}

void CameraUpdater::computeCameraVectors(const glm::quat& rotation, 
                                         glm::vec3& forward, 
                                         glm::vec3& up) const
{
    forward = rotation * glm::vec3(0, 0, -1);
    up = rotation * glm::vec3(0, 1, 0);
}

} // namespace RenderModule

