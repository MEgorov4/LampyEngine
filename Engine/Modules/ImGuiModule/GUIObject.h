#pragma once
#include <cstdint>

class GUIObject
{
    uint32_t m_objectID;
public:
    GUIObject();
    GUIObject(const GUIObject& T) = delete;
    GUIObject& operator=(const GUIObject& T) = delete;
    virtual ~GUIObject();
    
    virtual void render() {};

    uint32_t getID() const { return m_objectID; }

};
