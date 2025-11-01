#pragma once

#include <EngineMinimal.h>
#include <Modules/ImGuiModule/GUIObject.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/RenderModule/Abstract/ITexture.h>
#include <SDL3/SDL_events.h>
#include <Foundation/Event/Event.h>

namespace InputModule
{
class InputModule;
}

namespace RenderModule
{
class RenderModule;
struct TextureHandle;
} // namespace RenderModule

namespace EngineCore
{
    namespace Foundation
    {
    }
}

class GUIEditorViewport : public ImGUIModule::GUIObject
{
    RenderModule::RenderModule* m_renderModule;
    InputModule::InputModule* m_inputModule;
    ECSModule::ECSModule* m_ecsModule;

    RenderModule::TextureHandle m_offscreenImageDescriptor;
    
    Event<SDL_KeyboardEvent>::Subscription m_keySub;
    Event<SDL_MouseMotionEvent>::Subscription m_mouseSub;

    flecs::entity m_viewportEntity;
    glm::vec3 m_cameraPos   = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

    float m_cameraSpeed = 30.f;
    float m_deltaTime   = 0.f;
    bool m_firstMouse   = true;
    float m_lastX, m_lastY;

    bool m_processInput                = false;
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

    flecs::entity getViewportEntity();
};
