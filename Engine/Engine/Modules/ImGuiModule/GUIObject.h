#pragma once
#include <cstdint>
#include <string>

namespace ImGUIModule
{
    /// <summary>
    /// Represents a base class for GUI objects that can be rendered using ImGui.
    /// </summary>
    class GUIObject
    {
        uint32_t m_objectID; ///< Unique identifier for the GUI object.
        bool m_isVisible; ///< Visibility flag for the window.
        std::string m_windowName; ///< Display name of the window.

    public:
        /// <summary>
        /// Constructs a GUI object and assigns it a unique ID.
        /// </summary>
        GUIObject();

        /// <summary>
        /// Destroys the GUI object and removes it from the ImGui system.
        /// </summary>
        virtual ~GUIObject() = default;

        /// <summary>
        /// Virtual function for rendering the GUI object.
        /// Override this method in derived classes to implement custom rendering logic.
        /// </summary>
        virtual void render(float deltaTime) = 0;

        /// <summary>
        /// Retrieves the unique ID of the GUI object.
        /// </summary>
        /// <returns>The object's unique identifier.</returns>
        uint32_t getID() const { return m_objectID; }

        /// <summary>
        /// Sets the window name for this GUI object.
        /// </summary>
        void setWindowName(const std::string& name) { m_windowName = name; }

        /// <summary>
        /// Gets the window name for this GUI object.
        /// </summary>
        const std::string& getWindowName() const { return m_windowName; }

        /// <summary>
        /// Sets the visibility of the window.
        /// </summary>
        void setVisible(bool visible) { m_isVisible = visible; }

        /// <summary>
        /// Gets the visibility of the window.
        /// </summary>
        bool isVisible() const { return m_isVisible; }

        /// <summary>
        /// Shows the window.
        /// </summary>
        void show() { m_isVisible = true; }

        /// <summary>
        /// Hides the window.
        /// </summary>
        void hide() { m_isVisible = false; }

        /// <summary>
        /// Toggles the visibility of the window.
        /// </summary>
        void toggle() { m_isVisible = !m_isVisible; }
    };
}
