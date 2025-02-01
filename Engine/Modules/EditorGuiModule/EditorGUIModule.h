#pragma once
#include <memory>

#include "EditorToolPanel.h"
#include "MainMenuBar.h"
#include "OutputLog.h"
#include "ContentBrowser.h"

#include "../ImGuiModule/ImGuiModule.h"

class EditorGUIModule
{
	std::unique_ptr<GUIEditorToolPanel> m_toolPanel;
	std::unique_ptr<GUIMainMenuBar> m_mainMenuBar;
	std::unique_ptr<GUIOutputLog> m_outputLog;
	std::unique_ptr<GUIContentBrowser> m_contentBrowser;

public:
	EditorGUIModule() {}
	~EditorGUIModule() {}
	static EditorGUIModule& getInstance()
	{
		static EditorGUIModule EditorGUIModule;
		return EditorGUIModule;
	}

	void startUp()
	{
		LOG_INFO("EditorGUIModule: Startup");
		m_toolPanel = std::make_unique<GUIEditorToolPanel>();
		m_mainMenuBar = std::make_unique<GUIMainMenuBar>();
		m_outputLog = std::make_unique<GUIOutputLog>();
		m_contentBrowser = std::make_unique<GUIContentBrowser>();
	}
	
	void render()
	{
		ImGuiModule::getInstance().renderUI();
	};

	GUIMainMenuBar* getMenuBar()
	{
		return m_mainMenuBar.get();
	}

	void shutDown()
	{
		LOG_INFO("EditorGUIModule: Shut down");

		m_toolPanel.reset();
		m_mainMenuBar.reset();
		m_outputLog.reset();
		m_contentBrowser.reset();
	}
};