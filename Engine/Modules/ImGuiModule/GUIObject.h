#pragma once
#include <cstdint>

/// <summary>
/// Represents a base class for GUI objects that can be rendered using ImGui.
/// </summary>
class GUIObject
{
    uint32_t m_objectID; ///< Unique identifier for the GUI object.

public:
    /// <summary>
    /// Constructs a GUI object and assigns it a unique ID.
    /// </summary>
    GUIObject();

    /// <summary>
    /// Deleted copy constructor to prevent copying of GUI objects.
    /// </summary>
    GUIObject(const GUIObject& T) = delete;

    /// <summary>
    /// Deleted assignment operator to prevent copying.
    /// </summary>
    GUIObject& operator=(const GUIObject& T) = delete;

    /// <summary>
    /// Destroys the GUI object and removes it from the ImGui system.
    /// </summary>
    virtual ~GUIObject();

    /// <summary>
    /// Virtual function for rendering the GUI object.
    /// Override this method in derived classes to implement custom rendering logic.
    /// </summary>
    virtual void render() {};

    /// <summary>
    /// Retrieves the unique ID of the GUI object.
    /// </summary>
    /// <returns>The object's unique identifier.</returns>
    uint32_t getID() const { return m_objectID; }
};
