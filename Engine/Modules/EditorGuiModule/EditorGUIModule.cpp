#include "EditorGUIModule.h"


#include "../ImGuiModule/GUIObject.h"
#include "../ImGuiModule/ImGuiModule.h"
#include "../LoggerModule/Logger.h"
#include "../ProjectModule/ProjectModule.h"
#include "../LuaScriptModule/LuaScriptModule.h"
#include "../RenderModule/RenderModule.h"
#include "../InputModule/InputModule.h"
#include "../FilesystemModule/FilesystemModule.h"

#include "EditorConsole.h"
#include "EditorToolPanel.h"
#include "EditorViewport.h"
#include "MainMenuBar.h"
#include "OutputLog.h"

#include "ContentBrowser/ContentBrowser.h"
#include "WorldInspector/WorldInspector.h"


void EditorGUIModule::startup(const ModuleRegistry& registry)
{
    m_imGuiModule = std::dynamic_pointer_cast<ImGUIModule::ImGUIModule>(registry.getModule("ImGuiModule"));
    m_inputModule = std::dynamic_pointer_cast<InputModule::InputModule>(registry.getModule("InputModule"));
    m_projectModule = std::dynamic_pointer_cast<ProjectModule::ProjectModule>(registry.getModule("ProjectModule"));
    m_filesystemModule = std::dynamic_pointer_cast<FilesystemModule::FilesystemModule>(registry.getModule("FilesystemModule"));
    m_luaScriptModule = std::dynamic_pointer_cast<ScriptModule::LuaScriptModule>(registry.getModule("ScriptModule"));
    m_ecsModule = std::dynamic_pointer_cast<ECSModule::ECSModule>(registry.getModule("ECSModule"));
    m_logger = std::dynamic_pointer_cast<Logger::Logger>(registry.getModule("Logger"));
    m_renderModule = std::dynamic_pointer_cast<RenderModule::RenderModule>(registry.getModule("RenderModule"));

    m_logger->log(Logger::LogVerbosity::Info, "Startup", "EditorGUIModule");
    
    m_logger->log(Logger::LogVerbosity::Info, "Create GUI objects", "EditorGUIModule");
    
    m_imGuiModule->addObject(new GUIEditorToolPanel(m_ecsModule, m_projectModule));
    m_imGuiModule->addObject(new GUIMainMenuBar());
    m_imGuiModule->addObject(new GUIOutputLog(m_logger));
    m_imGuiModule->addObject(new GUIContentBrowser(m_filesystemModule, m_projectModule, m_ecsModule));
    m_imGuiModule->addObject(new GUIWorldInspector(m_projectModule, m_filesystemModule, m_ecsModule));
    m_imGuiModule->addObject(new GUIEditorViewport(m_logger, m_renderModule, m_inputModule, m_ecsModule));
    m_imGuiModule->addObject(new GUIEditorConsole(m_luaScriptModule));
}

void EditorGUIModule::shutdown()
{
    m_logger->log(Logger::LogVerbosity::Info, "Shutdown", "EditorGUIModule");
}


void EditorGUIModule::render(float deltaTime) const
{
	m_imGuiModule->renderUI(deltaTime);
}

