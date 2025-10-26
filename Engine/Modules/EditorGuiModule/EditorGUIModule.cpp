#include "EditorGUIModule.h"


#include <Modules/ImGuiModule/GUIObject.h>
#include <Modules/ImGuiModule/ImGuiModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/LuaScriptModule/LuaScriptModule.h>
#include <Modules/RenderModule/RenderModule.h>
#include <Modules/InputModule/InputModule.h>
#include <Modules/FilesystemModule/FilesystemModule.h>

#include "EditorConsole.h"
#include "EditorToolPanel.h"
#include "EditorViewport.h"
#include "MainMenuBar.h"
#include "OutputLog.h"

#include "ContentBrowser/ContentBrowser.h"
#include "WorldInspector/WorldInspector.h"

void EditorGUIModule::startup()
{
    m_imGuiModule = GCXM(ImGUIModule::ImGUIModule);

    LT_LOG(LogVerbosity::Info, "EditorGUIModule", "Startup");
    LT_LOG(LogVerbosity::Info, "EditorGUIModule", "Create GUI objects");

    m_imGuiModule->addObject(new GUIEditorToolPanel());
    m_imGuiModule->addObject(new GUIMainMenuBar());
    m_imGuiModule->addObject(new GUIOutputLog());
    m_imGuiModule->addObject(new GUIContentBrowser());
    m_imGuiModule->addObject(new GUIWorldInspector());
    m_imGuiModule->addObject(new GUIEditorViewport());
    m_imGuiModule->addObject(new GUIEditorConsole());
}

void EditorGUIModule::shutdown()
{
    LT_LOG(LogVerbosity::Info, "EditorGUIModule", "Shutdown");
}

void EditorGUIModule::render(float deltaTime) const
{
    m_imGuiModule->renderUI(deltaTime);
}
