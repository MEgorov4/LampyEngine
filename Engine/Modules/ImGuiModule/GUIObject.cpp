#include "GUIObject.h"

namespace ImGUIModule
{
    GUIObject::GUIObject() : m_isVisible(true), m_windowName("Unnamed Window")
    {
        static uint32_t ID = 0;
        m_objectID = ID++;
    }
}
