#include "EditorViewport.h"

#include "WorldInspector/WorldInspector.h"

#include <Modules/InputModule/InputModule.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/RenderModule/IRenderer.h>
#include <Modules/RenderModule/RenderModule.h>
//clang-format off
#include <imgui.h>
#include <Modules/ImGuiModule/ImGuizmo.h>
//clang-format on

flecs::entity GUIEditorViewport::getViewportEntity()
{
    auto& world = m_ecsModule->getCurrentWorld()->get();

    m_viewportEntity = world.lookup("ViewportCamera");

    return m_viewportEntity;
}

GUIEditorViewport::GUIEditorViewport() :
    ImGUIModule::GUIObject(), m_renderModule(GCM(RenderModule::RenderModule)),
    m_inputModule(GCM(InputModule::InputModule)), m_ecsModule(GCM(ECSModule::ECSModule)),
    m_viewportEntity(m_ecsModule->getCurrentWorld()->get().entity("ViewportCamera"))
{
    m_keySub   = m_inputModule->OnKeyboardEvent.subscribe(std::bind_front(&GUIEditorViewport::onKeyAction, this));
    m_mouseSub = m_inputModule->OnMouseMotionEvent.subscribe(std::bind_front(&GUIEditorViewport::onMouseAction, this));

    m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    m_cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
}

GUIEditorViewport::~GUIEditorViewport()
{
}

void GUIEditorViewport::render(float deltaTime)
{
    getViewportEntity();
    m_deltaTime = deltaTime;
    if (ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoScrollbar))
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();

        // 1. Отрисовываем рендер-таргет
        ImGui::Image(m_renderModule->getRenderer()->getOutputRenderHandle(avail.x, avail.y).id, avail, ImVec2(0, 1),
                     ImVec2(1, 0) // если текстура перевёрнута по Y
        );

        // --- вычисляем реальный экранный прямоугольник картинки ---
        ImVec2 imgMin = ImGui::GetItemRectMin();

        m_processInput = ImGui::IsWindowFocused() && ImGui::IsMouseDown(ImGuiMouseButton_Right);

        // 2. ImGuizmo
        ImGuizmo::AllowAxisFlip(false);
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(imgMin.x, imgMin.y, avail.x, avail.y);

        auto& world = m_ecsModule->getCurrentWorld()->get();
        if (auto ent = GUIWorldInspector::m_selectedEntity)
        {
            glm::vec3 pos{0.f};
            glm::quat rot{1.f, 0.f, 0.f, 0.f};
            glm::vec3 scl{1.f};

            if (const PositionComponent* pComp = ent.get<PositionComponent>())
                pos = pComp->toGLMVec();
            if (const RotationComponent* rComp = ent.get<RotationComponent>())
                rot = rComp->toQuat();
            if (const ScaleComponent* sComp = ent.get<ScaleComponent>())
                scl = sComp->toGLMVec();

            // 3. Собираем модельную матрицу
            glm::mat4 model(1.0f);
            model = glm::translate(model, pos);
            model *= glm::mat4_cast(rot);
            model = glm::scale(model, scl);

            // 4. Матрицы камеры — те же, что и в рендере
            const PositionComponent* cPosComp = m_viewportEntity.get<PositionComponent>();
            const RotationComponent* cRotComp = m_viewportEntity.get<RotationComponent>();
            const CameraComponent* camComp    = m_viewportEntity.get<CameraComponent>();

            float aspect = avail.x > 0 ? float(avail.x) / float(avail.y) : 1.0f;

            glm::mat4 proj = glm::perspective(glm::radians(camComp->fov), aspect, camComp->nearClip, camComp->farClip);
            glm::vec3 cameraPos = cPosComp->toGLMVec();
            glm::quat camRot    = cRotComp->toQuat();
            glm::vec3 forward   = camRot * glm::vec3(0, 0, -1);
            glm::vec3 up        = camRot * glm::vec3(0, 1, 0);

            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + forward, up);

            // 5. Гизмо
            static ImGuizmo::OPERATION gizmoOp = ImGuizmo::TRANSLATE;
            static ImGuizmo::MODE gizmoMode    = ImGuizmo::LOCAL;

            if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), gizmoOp, gizmoMode,
                                     glm::value_ptr(model)))
            {
                // robust TRS-деcompose
                glm::vec3 skew;
                glm::vec4 persp;
                glm::vec3 newS;
                glm::quat newR;
                glm::vec3 newT;
                glm::decompose(model, newS, newR, newT, skew, persp); // <glm/gtx/matrix_decompose.hpp>

                // Обновляем ECS компоненты
                ent.set<PositionComponent>({newT.x, newT.y, newT.z});

                if (auto* rc = ent.get_mut<RotationComponent>())
                {
                    rc->fromQuat(newR); // сохраняем в тех же Y->X->Z
                    ent.modified<RotationComponent>();
                }

                ent.set<ScaleComponent>({newS.x, newS.y, newS.z});
            }
            // Хоткеи для переключения операций
            if (ImGui::IsKeyPressed(ImGuiKey_T))
                gizmoOp = ImGuizmo::TRANSLATE;
            if (ImGui::IsKeyPressed(ImGuiKey_R))
                gizmoOp = ImGuizmo::ROTATE;
            if (ImGui::IsKeyPressed(ImGuiKey_S))
                gizmoOp = ImGuizmo::SCALE;
        }
    }
    ImGui::End();
}

void GUIEditorViewport::onKeyAction(SDL_KeyboardEvent event)
{
    if (!m_viewportEntity.is_valid())
        return;

    if (!m_processInput)
        return;
    m_cameraPos              = m_viewportEntity.get<PositionComponent>()->toGLMVec();
    glm::quat cameraRotation = m_viewportEntity.get<RotationComponent>()->toQuat();

    float speed = m_cameraSpeed;
    glm::vec3 movement(0.0f);
    glm::vec3 cameraFront = cameraRotation * glm::vec3(0, 0, -1);
    cameraFront           = glm::normalize(cameraFront);

    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, m_cameraUp));

    if (event.key == SDLK_W)
        movement += speed * cameraFront;
    if (event.key == SDLK_S)
        movement -= speed * cameraFront;

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
    glm::vec3 velocity{0.0f}; // хранить как член класса
    float accel   = 20.0f;
    float damping = 5.0f;

    glm::vec3 targetVel = movement / m_deltaTime; // какая скорость нужна
    velocity += (targetVel - velocity) * accel * m_deltaTime;
    velocity *= 1.0f / (1.0f + damping * m_deltaTime);

    m_cameraPos += velocity * m_deltaTime;
    m_viewportEntity.set<PositionComponent>({.x = m_cameraPos.x, .y = m_cameraPos.y, .z = m_cameraPos.z});
}

void GUIEditorViewport::onMouseAction(SDL_MouseMotionEvent mouseMotion)
{
    if (!m_viewportEntity.is_valid())
        return;
    if (!m_processInput)
    {
        m_firstMouse = true;
        return;
    }
    const RotationComponent* rotation = m_viewportEntity.get<RotationComponent>();

    float pitch = rotation->x;
    float yaw   = rotation->y;

    if (m_firstMouse)
    {
        m_lastX      = mouseMotion.x;
        m_lastY      = mouseMotion.y;
        m_firstMouse = false;
    }

    float xoffset = (mouseMotion.x - m_lastX);
    float yoffset = -(m_lastY - mouseMotion.y);
    m_lastX       = mouseMotion.x;
    m_lastY       = mouseMotion.y;

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

void GUIEditorViewport::drawGuizmo(int x, int y)
{
}

void GUIEditorViewport::drawGrid()
{
}
