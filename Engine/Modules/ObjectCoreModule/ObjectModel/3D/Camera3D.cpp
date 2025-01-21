#include "Camera3D.h"

Camera3D::Camera3D(Node* parentNode)
    : Node3D(parentNode), m_fov(60.f), m_aspectRatio(16.0f / 9.0f), m_nearPlane(0.1f), m_farPlane(1000.f)
{
    setPerspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
    updateViewMatrix();
}

void Camera3D::setPerspective(float fov, float aspect, float nearPlane, float farPlane)
{
    m_fov = fov;
    m_aspectRatio = aspect;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;

    m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

glm::mat4 Camera3D::getViewMatrix()
{
    return m_viewMatrix;
}

glm::mat4 Camera3D::getProjectionMatrix()
{
    return m_projectionMatrix;
}

void Camera3D::updateViewMatrix()
{
    glm::mat4 rotation = glm::yawPitchRoll(glm::radians(m_rotation.y), glm::radians(m_rotation.x), glm::radians(m_rotation.z));
    glm::vec3 forward = glm::normalize(glm::vec3(rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::vec3(rotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

    m_viewMatrix = glm::lookAt(m_position, m_position + forward, up);
}

