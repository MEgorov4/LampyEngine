#pragma once

#include <glm/vec3.hpp>
#include <SDL3/SDL_events.h>

#include "../ImGuiModule/GUIObject.h"
#include "../ObjectCoreModule/ECS/ECSModule.h"
#include "../RenderModule/Abstract/ITexture.h"

namespace InputModule
{
    class InputModule;
}

namespace RenderModule
{
    class RenderModule;
    struct TextureHandle;
}

class GUIEditorViewport : public ImGUIModule::GUIObject
{
    Logger::Logger* m_logger;
    RenderModule::RenderModule* m_renderModule;
    InputModule::InputModule* m_inputModule;
    ECSModule::ECSModule* m_ecsModule;

    RenderModule::TextureHandle m_offscreenImageDescriptor;

    flecs::entity m_viewportEntity;
    int m_keyActionHandlerID;
    int m_mouseActionHandlerID;

    glm::vec3 m_cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float m_cameraSpeed = 30.f;
    float m_deltaTime = 0.f;
    bool m_firstMouse = true;
    float m_lastX, m_lastY;

    bool m_processInput = false;
    static constexpr float aspectRatio = 16.0f / 9.0f;

public:
    GUIEditorViewport();
    virtual ~GUIEditorViewport() override;

    virtual void render(float deltaTime) override;

private:
    void onKeyAction(SDL_KeyboardEvent event);
    void onMouseAction(SDL_MouseMotionEvent mouseMotion);

    void drawGuizmo(int x, int y);
    void drawGrid();
};
