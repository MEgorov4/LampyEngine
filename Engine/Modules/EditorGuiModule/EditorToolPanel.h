#pragma once

#include <memory>

#include "../ImGuiModule/GUIObject.h"

namespace ProjectModule
{
    class ProjectModule;
}

namespace ECSModule
{
    class ECSModule;
}

class GUIEditorToolPanel : public ImGuiModule::GUIObject
{
    std::shared_ptr<ECSModule::ECSModule> m_ecsModule;
    std::shared_ptr<ProjectModule::ProjectModule> m_projectModule;

public:
    GUIEditorToolPanel(std::shared_ptr<ECSModule::ECSModule> ecsModule, std::shared_ptr<ProjectModule::ProjectModule> projectModule);
    virtual ~GUIEditorToolPanel() override = default;

    virtual void render() override;

private:
    void renderSaveWorldPopup();
};
