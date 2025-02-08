#pragma once
#include <memory>

#include "EditorToolPanel.h"
#include "MainMenuBar.h"
#include "OutputLog.h"
#include "ContentBrowser.h"
#include "EditorViewport.h"
#include "WorldInspector.h"
/// <summary>
/// Manages the graphical user interface (GUI) of the editor, including tool panels, menu bars, logs, and content browsers.
/// Implements a singleton pattern to ensure a single instance.
/// </summary>
class EditorGUIModule
{
    std::unique_ptr<GUIEditorToolPanel> m_toolPanel; ///< Unique pointer to the editor tool panel.
    std::unique_ptr<GUIMainMenuBar> m_mainMenuBar; ///< Unique pointer to the main menu bar.
    std::unique_ptr<GUIOutputLog> m_outputLog; ///< Unique pointer to the output log.
    std::unique_ptr<GUIContentBrowser> m_contentBrowser; ///< Unique pointer to the content browser.
    std::unique_ptr<GUIWorldInspector> m_worldInspector; ///< Unique pointer to the world inspector.
    std::unique_ptr<GUIEditorViewport> m_editorViewport; ///< Unique pointer to the viewport.

public:
    /// <summary>
    /// Retrieves the singleton instance of the EditorGUIModule.
    /// </summary>
    /// <returns>Reference to the singleton EditorGUIModule instance.</returns>
    static EditorGUIModule& getInstance()
    {
        static EditorGUIModule EditorGUIModule;
        return EditorGUIModule;
    }

    /// <summary>
    /// Initializes the editor GUI by creating tool panels, menus, logs, and content browsers.
    /// </summary>

    void startUp();
    /// <summary>
    /// Renders the GUI elements using the ImGui module.
    /// </summary>
    void render();

    /// <summary>
    /// Retrieves a pointer to the main menu bar instance.
    /// </summary>
    /// <returns>Pointer to the GUIMainMenuBar instance.</returns>
    GUIMainMenuBar* getMenuBar();

    /// <summary>
    /// Shuts down the editor GUI by releasing all GUI components.
    /// </summary>
    void shutDown();
};
