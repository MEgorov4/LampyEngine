#include "EditorViewport.h"
#include <imgui.h>
#include "../RenderModule/RenderModule.h"
#include "../InputModule/InputModule.h"

GUIEditorViewport::GUIEditorViewport()
    : GUIObject(),
    m_offscreenImageDescriptor(RenderModule::getInstance().getRenderer()->getOffscreenImageDescriptor()),
    m_viewportEntity(ECSModule::getInstance().getCurrentWorld().entity("ViewportCamera"))
{
    m_keyActionHandlerID = InputModule::getInstance().OnKeyAction.subscribe(std::bind_front(&GUIEditorViewport::onKeyAction, this));
    m_mouseActionHandlerID = InputModule::getInstance().OnMousePosAction.subscribe(std::bind_front(&GUIEditorViewport::onMouseAction, this));

    m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
}

GUIEditorViewport::~GUIEditorViewport()
{
    InputModule::getInstance().OnKeyAction.unsubscribe(m_keyActionHandlerID);
    InputModule::getInstance().OnMousePosAction.unsubscribe(m_mouseActionHandlerID);
}

void GUIEditorViewport::render()
{
    if (ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoScrollbar))
    {
        m_processInput = ImGui::IsWindowFocused() && ImGui::IsMouseDown(ImGuiMouseButton_Left);

        if (m_offscreenImageDescriptor)
        {
            ImGui::Image(m_offscreenImageDescriptor, ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 35));
        }
    }
    ImGui::End();
}

void GUIEditorViewport::onKeyAction(int code, int, int, int)
{
    if (!m_processInput)
        return;

    float speed = m_cameraSpeed;
    glm::vec3 movement(0.0f);

    glm::vec3 cameraRight = glm::normalize(glm::cross(m_cameraFront, m_cameraUp));

    if (code == 87) 
        movement += speed * m_cameraFront;
    if (code == 83) 
        movement -= speed * m_cameraFront;
    if (code == 65) 
        movement -= speed * cameraRight;
    if (code == 68) 
        movement += speed * cameraRight;
    if (code == 32) 
        movement += speed * m_cameraUp;
    if (code == 340) 
        movement -= speed * m_cameraUp;

    m_cameraPos += movement;

    if (m_viewportEntity.is_alive()) {
        m_viewportEntity.set<Position>({ m_cameraPos.x, m_cameraPos.y, m_cameraPos.z });
    }
}

void GUIEditorViewport::onMouseAction(double mouseX, double mouseY)
{
    if (!m_processInput)
        return;

    if (m_firstMouse) {
        m_lastX = mouseX;
        m_lastY = mouseY;
        m_firstMouse = false;
    }

    float xoffset = mouseX - m_lastX;
    float yoffset = m_lastY - mouseY; // Инверсия Y
    m_lastX = mouseX;
    m_lastY = mouseY;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    m_yaw += xoffset;
    m_pitch += yoffset;

    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    direction.y = sin(glm::radians(m_pitch));
    direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    m_cameraFront = glm::normalize(direction);


    if (m_viewportEntity.is_alive()) {
        m_viewportEntity.set<Rotation>({ m_pitch, m_yaw, 0.0f });
    }
}
