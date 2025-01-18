#pragma once

#include "Node3D.h"

class Camera3D : public Node3D
{
protected:
	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;

	float m_fov;
	float m_aspectRatio;
	float m_nearPlane;
	float m_farPlane;

public:
	Camera3D(Node* parentNode = nullptr);

	void setPerspective(float fov, float aspect, float nearPlane, float farPlane);
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();

	void updateViewMatrix(); 
};