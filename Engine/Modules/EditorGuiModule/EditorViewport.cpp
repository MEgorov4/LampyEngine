#include "EditorViewport.h"
#include <imgui.h>

#include "../LoggerModule/Logger.h"
#include "../RenderModule/RenderModule.h"
#include "../InputModule/InputModule.h"
#include "../RenderModule/IRenderer.h"

#include "../ObjectCoreModule/ECS/ECSComponents.h"

GUIEditorViewport::GUIEditorViewport(const std::shared_ptr<Logger::Logger>& logger,
                                     const std::shared_ptr<RenderModule::RenderModule>& renderModule,
                                     const std::shared_ptr<InputModule::InputModule>& inputModule,
                                     const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
    : ImGUIModule::GUIObject()
      , m_logger(logger)
      , m_renderModule(renderModule)
      , m_inputModule(inputModule)
      , m_ecsModule(ecsModule)
      , m_viewportEntity(m_ecsModule->getCurrentWorld().entity("ViewportCamera"))
{
    m_keyActionHandlerID = m_inputModule->OnKeyboardEvent.subscribe(
        std::bind_front(&GUIEditorViewport::onKeyAction, this));
    m_mouseActionHandlerID = m_inputModule->OnMouseMotionEvent.subscribe(
        std::bind_front(&GUIEditorViewport::onMouseAction, this));

    m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
}

GUIEditorViewport::~GUIEditorViewport()
{
    m_inputModule->OnKeyboardEvent.unsubscribe(m_keyActionHandlerID);
    m_inputModule->OnMouseMotionEvent.unsubscribe(m_mouseActionHandlerID);
}

void GUIEditorViewport::render(float deltaTime)
{
    m_deltaTime = deltaTime;
    if (ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoScrollbar))
    {
        m_offscreenImageDescriptor = m_renderModule->getRenderer()->getOutputRenderHandle();
        ImGui::Image(m_offscreenImageDescriptor.id, ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 36));
        m_processInput = ImGui::IsWindowFocused() && ImGui::IsMouseDown(ImGuiMouseButton_Right);
    }
    ImGui::End();
}

void GUIEditorViewport::onKeyAction(SDL_KeyboardEvent event)
{
    m_logger->log(Logger::LogVerbosity::Info,
                  "Key action catch: " + std::to_string(event.key) + '\n' + "Current delta time: " + std::to_string(
                      m_deltaTime),
                  "EditorGUIModule_GUIEditorViewport");
    if (!m_processInput)
        return;
    m_cameraPos = m_viewportEntity.get<PositionComponent>()->toGLMVec();
    glm::quat cameraRotation = m_viewportEntity.get<RotationComponent>()->toQuat();

    float speed = m_cameraSpeed;
    glm::vec3 movement(0.0f);
    glm::vec3 cameraFront = cameraRotation * glm::vec3(0, 0, -1);
    cameraFront = glm::normalize(cameraFront);

    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, m_cameraUp));

    if (event.key == SDLK_W)
        movement -= speed * cameraFront;
    if (event.key == SDLK_S)
        movement += speed * cameraFront;

    if (event.key == SDLK_A)
        movement -= speed * cameraRight;
    if (event.key == SDLK_D)
        movement += speed * cameraRight;

    glm::vec3 cameraUp = cameraRotation * glm::vec4(0, 1, 0, 1);
    if (event.key == SDLK_SPACE)
        movement += speed * cameraUp;
    if (event.key == SDL_KMOD_CTRL)
        movement -= speed * cameraUp;

    glm::vec3 newCameraPos = m_cameraPos + movement;
    m_cameraPos = glm::mix(m_cameraPos, newCameraPos, 1000 * m_deltaTime);
    if (m_viewportEntity.is_alive())
    {
        m_viewportEntity.set<PositionComponent>({.x = m_cameraPos.x, .y = m_cameraPos.y, .z = m_cameraPos.z});
    }
}


void GUIEditorViewport::onMouseAction(SDL_MouseMotionEvent mouseMotion)
{
    if (!m_processInput)
    {
        m_firstMouse = true;
        return;
    }

    const RotationComponent* rotation = m_viewportEntity.get<RotationComponent>();

    float pitch = rotation->x;
    float yaw = rotation->y;

    if (m_firstMouse)
    {
        m_lastX = mouseMotion.x;
        m_lastY = mouseMotion.y;
        m_firstMouse = false;
    }

    float xoffset = (mouseMotion.x - m_lastX);
    float yoffset = -(m_lastY - mouseMotion.y);
    m_lastX = mouseMotion.x;
    m_lastY = mouseMotion.y;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (m_viewportEntity.is_alive())
    {
        m_viewportEntity.set<RotationComponent>({pitch, yaw, rotation->z});
    }
}
