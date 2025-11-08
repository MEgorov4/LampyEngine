#pragma once

#include <EngineMinimal.h>
#include <Modules/ImGuiModule/GUIObject.h>

namespace ProjectModule
{
class ProjectModule;
}

namespace ECSModule
{
class ECSModule;
}

class GUIEditorToolPanel : public ImGUIModule::GUIObject
{
    ECSModule::ECSModule* m_ecsModule;
    ProjectModule::ProjectModule* m_projectModule;

  public:
    GUIEditorToolPanel();
    virtual ~GUIEditorToolPanel() override = default;

    virtual void render(float deltaTime) override;

  private:
    void renderSaveWorldPopup();
};
