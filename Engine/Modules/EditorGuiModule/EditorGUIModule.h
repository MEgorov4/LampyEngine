#pragma once
#include <memory>

#include "EditorToolPanel.h"
#include "MainMenuBar.h"
#include "OutputLog.h"
#include "ContentBrowser.h"

#include "../ImGuiModule/ImGuiModule.h"

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
    void startUp()
    {
        LOG_INFO("EditorGUIModule: Startup");
        m_toolPanel = std::make_unique<GUIEditorToolPanel>();
        m_mainMenuBar = std::make_unique<GUIMainMenuBar>();
        m_outputLog = std::make_unique<GUIOutputLog>();
        m_contentBrowser = std::make_unique<GUIContentBrowser>();
    }

    /// <summary>
    /// Renders the GUI elements using the ImGui module.
    /// </summary>
    void render()
    {
        ImGuiModule::getInstance().renderUI();
    }

    /// <summary>
    /// Retrieves a pointer to the main menu bar instance.
    /// </summary>
    /// <returns>Pointer to the GUIMainMenuBar instance.</returns>
    GUIMainMenuBar* getMenuBar()
    {
        return m_mainMenuBar.get();
    }

    /// <summary>
    /// Shuts down the editor GUI by releasing all GUI components.
    /// </summary>
    void shutDown()
    {
        LOG_INFO("EditorGUIModule: Shut down");

        m_toolPanel.reset();
        m_mainMenuBar.reset();
        m_outputLog.reset();
        m_contentBrowser.reset();
    }
};
