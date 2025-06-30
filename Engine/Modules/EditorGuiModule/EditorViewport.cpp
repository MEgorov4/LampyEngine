#include "EditorViewport.h"
#include <imgui.h>
#include "../RenderModule/RenderModule.h"
#include "../InputModule/InputModule.h"
#include "../RenderModule/IRenderer.h"
#include "../ObjectCoreModule/ECS/ECSComponents.h"

GUIEditorViewport::GUIEditorViewport(const std::shared_ptr<RenderModule::RenderModule>& renderModule,
                                     const std::shared_ptr<InputModule::InputModule>& inputModule,
                                     const std::shared_ptr<ECSModule::ECSModule>& ecsModule) 
    : ImGuiModule::GUIObject(), m_renderModule(renderModule), m_inputModule(inputModule), m_ecsModule(ecsModule),
    m_offscreenImageDescriptor(m_renderModule->getRenderer()->getOffscreenImageDescriptor()),
    m_viewportEntity(m_ecsModule->getCurrentWorld().entity("ViewportCamera"))
{
    m_keyActionHandlerID = m_inputModule->OnKeyAction.subscribe(std::bind_front(&GUIEditorViewport::onKeyAction, this));
    m_mouseActionHandlerID = m_inputModule->OnMousePosAction.subscribe(std::bind_front(&GUIEditorViewport::onMouseAction, this));

    m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
}

GUIEditorViewport::~GUIEditorViewport()
{
    m_inputModule->OnKeyAction.unsubscribe(m_keyActionHandlerID);
    m_inputModule->OnMousePosAction.unsubscribe(m_mouseActionHandlerID);
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
    m_cameraPos = m_viewportEntity.get<PositionComponent>()->toGLMVec();
    glm::quat cameraRotation = m_viewportEntity.get<RotationComponent>()->toQuat(); 

    float speed = m_cameraSpeed;
    glm::vec3 movement(0.0f);

    glm::vec3 cameraFront = cameraRotation * glm::vec3(0, 0, -1);
    cameraFront = glm::normalize(cameraFront);

    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, m_cameraUp));

    if (code == 87) // W
        movement -= speed * cameraFront;
    if (code == 83) // S
        movement += speed * cameraFront;

    if (code == 65) // A
        movement -= speed * cameraRight;
    if (code == 68) // D
        movement += speed * cameraRight;

    glm::vec3 cameraUp = cameraRotation * glm::vec3(0, 1, 0);
    if (code == 32)  
        movement += speed * cameraUp;
    if (code == 340) 
        movement -= speed * cameraUp;

    m_cameraPos += movement;

    if (m_viewportEntity.is_alive()) {
        m_viewportEntity.set<PositionComponent>({ m_cameraPos.x, m_cameraPos.y, m_cameraPos.z });
    }
}


void GUIEditorViewport::onMouseAction(double mouseX, double mouseY)
{
    if (!m_processInput) {
        m_firstMouse = true;
        return;
    }

    const RotationComponent* rotation = m_viewportEntity.get<RotationComponent>();

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
        m_viewportEntity.set<RotationComponent>({ pitch, yaw, rotation->z });
    }
}
