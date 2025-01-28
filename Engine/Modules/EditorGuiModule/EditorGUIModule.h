#pragma once
#include <memory>
#include "EditorToolPanel.h"
#include "MainMenuBar.h"
#include "../ImGuiModule/ImGuiModule.h"

class EditorGUIModule
{
	std::unique_ptr<GUIEditorToolPanel> m_toolPanel;
	std::unique_ptr<GUIMainMenuBar> m_mainMenuBar;
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
		m_toolPanel = std::make_unique<GUIEditorToolPanel>();
		m_mainMenuBar = std::make_unique<GUIMainMenuBar>();
	}

	void render()
	{
		ImGuiModule::getInstance().renderUI();
	};

	void shutDown()
	{
		m_toolPanel.reset();
		m_mainMenuBar.reset();
	}
};