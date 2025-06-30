#pragma once

#include <glm/vec3.hpp>

#include "../ImGuiModule/GUIObject.h"
#include "../ObjectCoreModule/ECS/ECSModule.h"

namespace InputModule
{
    class InputModule;
}

namespace RenderModule
{
    class RenderModule;
}

class GUIEditorViewport : public ImGuiModule::GUIObject
{
    std::shared_ptr<RenderModule::RenderModule> m_renderModule;
    std::shared_ptr<InputModule::InputModule> m_inputModule;
    std::shared_ptr<ECSModule::ECSModule> m_ecsModule;

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
    GUIEditorViewport(const std::shared_ptr<RenderModule::RenderModule>& renderModule,
                      const std::shared_ptr<InputModule::InputModule>& inputModule,
                      const std::shared_ptr<ECSModule::ECSModule>& ecsModule);
    virtual ~GUIEditorViewport() override;

    virtual void render() override;

private:
    void onKeyAction(int code, int a, int b, int c);
    void onMouseAction(double mouseX, double mouseY);
};
