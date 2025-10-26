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
namespace FilesystemModule
{
    class FilesystemModule;
}

class GUIEditorToolPanel : public ImGUIModule::GUIObject
{
    ECSModule::ECSModule* m_ecsModule;
    ProjectModule::ProjectModule* m_projectModule;
    FilesystemModule::FilesystemModule* m_filesystemModule;

public:
    GUIEditorToolPanel();
    virtual ~GUIEditorToolPanel() override = default;

    virtual void render(float deltaTime) override;

private:
    void renderSaveWorldPopup();
};
