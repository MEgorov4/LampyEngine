#pragma once 
#include <vector>

class GUIObject;
/// <summary>
/// Manages the ImGui user interface system, handling GUI object registration and rendering.
/// Implements a singleton pattern to ensure a single instance.
/// </summary>
class ImGuiModule
{
    std::vector<GUIObject*> m_GuiObjects; ///< List of registered GUI objects.

    /// <summary>
    /// Private constructor to enforce singleton pattern.
    /// </summary>
    ImGuiModule() = default;

public:
    /// <summary>
    /// Destructor for the ImGuiModule.
    /// </summary>
    ~ImGuiModule() = default;

    /// <summary>
    /// Retrieves the singleton instance of the ImGuiModule.
    /// </summary>
    /// <returns>Reference to the singleton ImGuiModule instance.</returns>
    void startup();
    void shutDown();
    static ImGuiModule& getInstance()
    {
        static ImGuiModule ImGuiModule;
        return ImGuiModule;
    }

    /// <summary>
    /// Renders the ImGui user interface.
    /// Calls the render function of all registered GUI objects.
    /// </summary>
    void renderUI() const;

    /// <summary>
    /// Registers a GUI object to be rendered.
    /// </summary>
    /// <param name="object">Pointer to the GUI object to be added.</param>
    void addObject(GUIObject* object);

    /// <summary>
    /// Removes a GUI object from the list using its unique ID.
    /// </summary>
    /// <param name="id">The unique ID of the GUI object to remove.</param>
    void removeObject(uint32_t id);
};
