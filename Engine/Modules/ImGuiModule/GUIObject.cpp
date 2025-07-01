#include "GUIObject.h"

namespace ImGUIModule
{
    GUIObject::GUIObject()
    {
        static uint32_t ID = 0;
        m_objectID = ID++;
    }
}
