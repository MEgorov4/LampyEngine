#pragma once
#include <flecs.h>
#include <memory>

#include <Modules/ImGuiModule/GUIObject.h> 


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
   ECSModule::ECSModule* m_ecsModule;
    
   flecs::world& m_world;

public:
    static flecs::entity m_selectedEntity;
    GUIWorldInspector();
    virtual ~GUIWorldInspector() override = default;

    virtual void render(float deltaTime) override;

private:
    void renderEntityTreePopup();
    void renderEntityTree();
    void renderSelectedEntityDefaults();
    void renderAddComponentPopup();
};
