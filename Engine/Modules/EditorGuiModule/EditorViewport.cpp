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
        m_processInput = ImGui::IsWindowFocused() && ImGui::IsMouseDown(ImGuiMouseButton_Right);

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

    m_cameraPos = m_viewportEntity.get<Position>()->toGLMVec();
    glm::quat cameraRotation = m_viewportEntity.get<Rotation>()->toQuat(); 

    float speed = m_cameraSpeed;
    glm::vec3 movement(0.0f);

    // Направление вперед (куда смотрит камера)
    glm::vec3 cameraFront = cameraRotation * glm::vec3(0, 0, -1);
    cameraFront = glm::normalize(cameraFront);

    // Направление вправо (перпендикулярное вперед и вверх)
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, m_cameraUp));

    // Движение вперед/назад относительно направления камеры
    if (code == 87) // W
        movement -= speed * cameraFront;
    if (code == 83) // S
        movement += speed * cameraFront;

    // Движение влево/вправо относительно поворота камеры
    if (code == 65) // A
        movement -= speed * cameraRight;
    if (code == 68) // D
        movement += speed * cameraRight;

    // Движение вверх/вниз относительно локального Up камеры
    glm::vec3 cameraUp = cameraRotation * glm::vec3(0, 1, 0);
    if (code == 32)  // Space (вверх)
        movement += speed * cameraUp;
    if (code == 340) // Shift (вниз)
        movement -= speed * cameraUp;

    // Применяем движение
    m_cameraPos += movement;

    if (m_viewportEntity.is_alive()) {
        m_viewportEntity.set<Position>({ m_cameraPos.x, m_cameraPos.y, m_cameraPos.z });
    }
}


void GUIEditorViewport::onMouseAction(double mouseX, double mouseY)
{
    if (!m_processInput) {
        m_firstMouse = true;
        return;
    }

    const Rotation* rotation = m_viewportEntity.get<Rotation>();

    float pitch = rotation->x;
    float yaw = rotation->y;

    if (m_firstMouse) {
        m_lastX = mouseX;
        m_lastY = mouseY;
        m_firstMouse = false;
    }

    float xoffset = (mouseX - m_lastX);
    float yoffset = -(m_lastY - mouseY); 
    m_lastX = mouseX;
    m_lastY = mouseY;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (m_viewportEntity.is_alive()) {
        m_viewportEntity.set<Rotation>({ pitch, yaw, rotation->z });
    }
}
