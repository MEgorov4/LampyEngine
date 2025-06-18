#include "EditorGUIModule.h"


#include "../ImGuiModule/ImGuiModule.h"
#include "../LoggerModule/Logger.h"

void EditorGUIModule::startup()
{
    ImGuiModule::getInstance().startup();
    LOG_INFO("EditorGUIModule: Startup");
    m_toolPanel = std::make_unique<GUIEditorToolPanel>();
    m_mainMenuBar = std::make_unique<GUIMainMenuBar>();
    m_outputLog = std::make_unique<GUIOutputLog>();
    m_contentBrowser = std::make_unique<GUIContentBrowser>();
    m_worldInspector = std::make_unique<GUIWorldInspector>();
    m_viewport = std::make_unique<GUIEditorViewport>();
    m_console = std::make_unique<GUIEditorConsole>();
    //m_scriptPanel = std::make_unique<GUIEditorScriptPanel>();
}

void EditorGUIModule::render()
{
	ImGuiModule::getInstance().renderUI();
}

GUIMainMenuBar* EditorGUIModule::getMenuBar()
{
    return m_mainMenuBar.get();
}

void EditorGUIModule::shutDown()
{
    LOG_INFO("EditorGUIModule: Shut down");

    m_toolPanel.reset();
    m_mainMenuBar.reset();
    m_outputLog.reset();
    m_contentBrowser.reset();
    m_worldInspector.reset();
    m_viewport.reset();
    m_console.reset();
    //m_scriptPanel.reset();

    ImGuiModule::getInstance().shutDown();
}
