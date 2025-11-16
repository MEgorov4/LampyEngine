#include "EditorViewport.h"

#include "WorldInspector/WorldInspector.h"

#include <Modules/InputModule/InputModule.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ScriptModule/LuaScriptModule.h>
#include <Modules/RenderModule/IRenderer.h>
#include <Modules/RenderModule/RenderLocator.h>
#include <Modules/RenderModule/RenderModule.h>
#include <Modules/TimeModule/TimeModule.h>
//clang-format off
#include <imgui.h> /////////////////////////
/////////////////////

#include <Modules/ImGuiModule/ImGuizmo.h>
//clang-format on

flecs::entity GUIEditorViewport::getViewportEntity()
{
    auto &world = m_ecsModule->getCurrentWorld()->get();

    m_viewportEntity = world.lookup("ViewportCamera");

    return m_viewportEntity;
}

GUIEditorViewport::GUIEditorViewport()
    : ImGUIModule::GUIObject(), m_renderModule(GCM(RenderModule::RenderModule)),
      m_inputModule(GCM(InputModule::InputModule)), m_ecsModule(GCM(ECSModule::ECSModule)),
      m_viewportEntity(m_ecsModule->getCurrentWorld()->get().entity("ViewportCamera"))
{
    m_keySub = m_inputModule->OnKeyboardEvent.subscribe(std::bind_front(&GUIEditorViewport::onKeyAction, this));
    m_mouseSub = m_inputModule->OnMouseMotionEvent.subscribe(std::bind_front(&GUIEditorViewport::onMouseAction, this));

    m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

   //GCM(ScriptModule::LuaScriptModule)
   //     ->callDevScript("Scripts/Dev/viewport_camera.lua", "setup", m_ecsModule->getCurrentWorld()->get());
}

GUIEditorViewport::~GUIEditorViewport()
{
}

void GUIEditorViewport::render(float deltaTime)
{
    ZoneScopedN("GUIObject::Viewport");
    getViewportEntity();
    m_deltaTime = GCM(TimeModule::TimeModule)->getDeltaTime();
    if (!isVisible())
        return;

    using namespace RenderModule;

    bool windowOpen = true;
    if (ImGui::Begin("Viewport", &windowOpen, ImGuiWindowFlags_NoScrollbar))
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();

        if (avail.x >= 1.0f && avail.y >= 1.0f)
        {
            ImGui::Image(m_renderModule->getRenderer()
                             ->getOutputRenderHandle(static_cast<int>(avail.x), static_cast<int>(avail.y))
                             .id,
                         avail, ImVec2(0, 1), ImVec2(1, 0)
            );
        }
        else
        {
            ImGui::Image(m_renderModule->getRenderer()->getOutputRenderHandle(1, 1).id, avail, ImVec2(0, 1),
                         ImVec2(1, 0));
        }

        ImVec2 imgMin = ImGui::GetItemRectMin();

        m_processInput = ImGui::IsWindowFocused() && ImGui::IsMouseDown(ImGuiMouseButton_Right);

        // 2. ImGuizmo
        ImGuizmo::AllowAxisFlip(false);
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(imgMin.x, imgMin.y, avail.x, avail.y);

        auto &world = m_ecsModule->getCurrentWorld()->get();
        if (auto ent = GUIWorldInspector::m_selectedEntity)
        {
            glm::vec3 pos{0.f};
            glm::quat rot{1.f, 0.f, 0.f, 0.f};
            glm::vec3 scl{1.f};

            if (const TransformComponent *transform = ent.get<TransformComponent>())
            {
                pos = transform->position.toGLMVec();
                rot = transform->rotation.toQuat();
                scl = transform->scale.toGLMVec();
            }

            glm::mat4 model(1.0f);
            model = glm::translate(model, pos);
            model *= glm::mat4_cast(rot);
            model = glm::scale(model, scl);

            const TransformComponent *cameraTransform = m_viewportEntity.get<TransformComponent>();
            const CameraComponent *camComp = m_viewportEntity.get<CameraComponent>();

            float aspect = avail.x > 0 ? float(avail.x) / float(avail.y) : 1.0f;

            glm::mat4 proj = glm::perspective(glm::radians(camComp->fov), aspect, camComp->nearClip, camComp->farClip);
            glm::vec3 cameraPos = cameraTransform ? cameraTransform->position.toGLMVec() : glm::vec3(0.0f);
            glm::quat camRot = cameraTransform ? cameraTransform->rotation.toQuat() : glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            glm::vec3 forward = camRot * glm::vec3(0, 0, -1);
            glm::vec3 up = camRot * glm::vec3(0, 1, 0);

            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + forward, up);

            static ImGuizmo::OPERATION gizmoOp = ImGuizmo::TRANSLATE;
            static ImGuizmo::MODE gizmoMode = ImGuizmo::LOCAL;

            if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), gizmoOp, gizmoMode,
                                     glm::value_ptr(model)))
            {
                glm::vec3 skew;
                glm::vec4 persp;
                glm::vec3 newS;
                glm::quat newR;
                glm::vec3 newT;
                glm::decompose(model, newS, newR, newT, skew, persp); // <glm/gtx/matrix_decompose.hpp>

                auto& mutableTransform = EnsureTransformComponent(ent);
                mutableTransform.position = {.x = newT.x, .y = newT.y, .z = newT.z};
                mutableTransform.scale = {.x = newS.x, .y = newS.y, .z = newS.z};
                mutableTransform.rotation.fromQuat(newR);
                ent.modified<TransformComponent>();
            }
            if (ImGui::IsKeyPressed(ImGuiKey_T))
                gizmoOp = ImGuizmo::TRANSLATE;
            if (ImGui::IsKeyPressed(ImGuiKey_R))
                gizmoOp = ImGuizmo::ROTATE;
            if (ImGui::IsKeyPressed(ImGuiKey_S))
                gizmoOp = ImGuizmo::SCALE;
        }
    }

    if (!windowOpen)
    {
        hide();
    }

    ImGui::End();
}

void GUIEditorViewport::onKeyAction(SDL_KeyboardEvent event)
{
    //GCM(ScriptModule::LuaScriptModule)->callDevScript("Scripts/Dev/viewport_camera.lua", "key_action", event);
    if (!m_viewportEntity.is_valid())
        return;

    if (!m_processInput)
        return;
    const TransformComponent* cameraTransform = m_viewportEntity.get<TransformComponent>();
    if (!cameraTransform)
        return;

    m_cameraPos = cameraTransform->position.toGLMVec();
    glm::quat cameraRotation = cameraTransform->rotation.toQuat();

    float speed = m_cameraSpeed;
    glm::vec3 movement(0.0f);
    glm::vec3 cameraFront = cameraRotation * glm::vec3(0, 0, -1);
    cameraFront = glm::normalize(cameraFront);

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
    glm::vec3 velocity{0.0f};
    float accel = 20.0f;
    float damping = 5.0f;

    glm::vec3 targetVel = movement / m_deltaTime;
    velocity += (targetVel - velocity) * accel * m_deltaTime;
    velocity *= 1.0f / (1.0f + damping * m_deltaTime);

    m_cameraPos += velocity * m_deltaTime;
    auto& mutableTransform = EnsureTransformComponent(m_viewportEntity);
    mutableTransform.position = {.x = m_cameraPos.x, .y = m_cameraPos.y, .z = m_cameraPos.z};
    m_viewportEntity.modified<TransformComponent>();
}

void GUIEditorViewport::onMouseAction(SDL_MouseMotionEvent mouseMotion)
{
    //GCM(ScriptModule::LuaScriptModule)->callDevScript("Scripts/Dev/viewport_camera.lua", "mouse_action", mouseMotion);
    if (!m_viewportEntity.is_valid())
        return;
    if (!m_processInput)
    {
        m_firstMouse = true;
        return;
    }
    const TransformComponent *transform = m_viewportEntity.get<TransformComponent>();
    if (!transform)
        return;

    const RotationComponent &rotation = transform->rotation;

    float pitch = rotation.x;
    float yaw = rotation.y;

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
        RotationComponent newRot;
        newRot.fromEulerDegrees(glm::vec3(pitch, yaw, rotation.z));
        auto& mutableTransform = EnsureTransformComponent(m_viewportEntity);
        mutableTransform.rotation = newRot;
        m_viewportEntity.modified<TransformComponent>();
    }
}

void GUIEditorViewport::drawGuizmo(int x, int y)
{
}

void GUIEditorViewport::drawGrid()
{
}
