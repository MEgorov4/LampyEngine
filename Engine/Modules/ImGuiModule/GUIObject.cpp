#include "GUIObject.h"

namespace ImGuiModule
{
    GUIObject::GUIObject()
    {
        static uint32_t ID = 0;
        m_objectID = ID++;
    }
}
