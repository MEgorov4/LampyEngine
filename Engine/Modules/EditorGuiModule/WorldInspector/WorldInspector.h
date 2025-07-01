#pragma once
#include <flecs.h>
#include <memory>

#include "../../ImGuiModule/GUIObject.h"


namespace FilesystemModule
{
    class FilesystemModule;
}

namespace ProjectModule
{
    class ProjectModule;
}

namespace ECSModule
{
    class ECSModule;
}

class GUIWorldInspector : public ImGUIModule::GUIObject
{
    std::shared_ptr<ProjectModule::ProjectModule> m_projectModule;
    std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
    std::shared_ptr<ECSModule::ECSModule> m_ecsModule;
    
    flecs::world& m_world;
    flecs::entity m_selectedEntity;

public:
    GUIWorldInspector(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                      const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                      const std::shared_ptr<ECSModule::ECSModule>& ecsModule);
    virtual ~GUIWorldInspector() override = default;

    virtual void render(float deltaTime) override;

private:
    void renderEntityTreePopup();
    void renderEntityTree();
    void renderSelectedEntityDefaults();
    void renderAddComponentPopup();
};
