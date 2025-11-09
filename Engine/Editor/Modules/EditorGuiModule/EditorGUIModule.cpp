#include "EditorGUIModule.h"

#include "AssetBrowser/AssetBrowser.h"
#include "ContentBrowser/ContentBrowser.h"
#include "EditorConsole.h"
#include "EditorToolPanel.h"
#include "EditorViewport.h"
#include "MainMenuBar.h"
#include "OutputLog.h"
#include "WorldInspector/WorldInspector.h"

#include <Modules/ImGuiModule/GUIObject.h>
#include <Modules/ImGuiModule/ImGuiModule.h>
#include <Modules/InputModule/InputModule.h>
#include <Modules/LuaScriptModule/LuaScriptModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/RenderModule/RenderModule.h>

void EditorGUIModule::startup()
{
    ZoneScopedN("EditorGUIModule::startup");
    m_imGuiModule = GCXM(ImGUIModule::ImGUIModule);

    LT_LOG(LogVerbosity::Info, "EditorGUIModule", "Startup");
    LT_LOG(LogVerbosity::Info, "EditorGUIModule", "Create GUI objects");

    // Register windows with their names
    auto toolPanel = m_imGuiModule->addObject(new GUIEditorToolPanel());
    if (auto ptr = toolPanel.lock()) {
        ptr->setWindowName("Tool Panel");
        m_windowObjects.push_back(toolPanel);
        m_windowNames.push_back("Tool Panel");
    }

    auto menuBar = m_imGuiModule->addObject(new GUIMainMenuBar());
    // Menu bar is always visible, don't add to window list
    
    auto outputLog = m_imGuiModule->addObject(new GUIOutputLog());
    if (auto ptr = outputLog.lock()) {
        ptr->setWindowName("Output Log");
        m_windowObjects.push_back(outputLog);
        m_windowNames.push_back("Output Log");
    }

    auto contentBrowser = m_imGuiModule->addObject(new GUIContentBrowser());
    if (auto ptr = contentBrowser.lock()) {
        ptr->setWindowName("Content Browser");
        m_windowObjects.push_back(contentBrowser);
        m_windowNames.push_back("Content Browser");
    }

    auto worldInspector = m_imGuiModule->addObject(new GUIWorldInspector());
    if (auto ptr = worldInspector.lock()) {
        ptr->setWindowName("World Inspector");
        m_windowObjects.push_back(worldInspector);
        m_windowNames.push_back("World Inspector");
    }

    auto viewport = m_imGuiModule->addObject(new GUIEditorViewport());
    if (auto ptr = viewport.lock()) {
        ptr->setWindowName("Viewport");
        m_windowObjects.push_back(viewport);
        m_windowNames.push_back("Viewport");
    }

    auto console = m_imGuiModule->addObject(new GUIEditorConsole());
    if (auto ptr = console.lock()) {
        ptr->setWindowName("Console");
        m_windowObjects.push_back(console);
        m_windowNames.push_back("Console");
    }

    auto assetBrowser = m_imGuiModule->addObject(new GUIAssetBrowser());
    if (auto ptr = assetBrowser.lock()) {
        ptr->setWindowName("Asset Browser");
        m_windowObjects.push_back(assetBrowser);
        m_windowNames.push_back("Asset Browser");
    }
}

void EditorGUIModule::shutdown()
{
    ZoneScopedN("EditorGUIModule::shutdown");
    LT_LOG(LogVerbosity::Info, "EditorGUIModule", "Shutdown");
}

void EditorGUIModule::render(float deltaTime) const
{
    ZoneScopedN("EditorGUIModule::render");
    m_imGuiModule->renderUI(deltaTime);
}
