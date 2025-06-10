#pragma once

#include "../ImGuiModule/GUIObject.h"
#include "../ObjectCoreModule/ECS/ECSModule.h"

class GUIEditorViewport : GUIObject
{
	void* m_offscreenImageDescriptor;

	flecs::entity m_viewportEntity;
	int m_keyActionHandlerID;
	int m_mouseActionHandlerID;

	glm::vec3 m_cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float m_cameraSpeed = 0.10f;

	bool m_firstMouse = true;
	double m_lastX, m_lastY;

	bool m_processInput = false;
public:
	GUIEditorViewport();
	virtual ~GUIEditorViewport() override;

	virtual void render() override;

private:
	void onKeyAction(int code, int a, int b, int c);
	void onMouseAction(double mouseX, double mouseY);
};
