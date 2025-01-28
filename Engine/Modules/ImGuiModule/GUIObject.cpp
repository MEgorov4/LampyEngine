#include "GUIObject.h"
#include "ImGuiModule.h"

GUIObject::GUIObject()
{
    static uint32_t ID = 0;
    m_objectID = ID++;

    ImGuiModule::getInstance().addObject(this);
}

GUIObject::~GUIObject()
{
    ImGuiModule::getInstance().removeObject(m_objectID);
}
