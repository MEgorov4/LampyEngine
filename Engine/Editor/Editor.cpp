#include "Editor.h"
#include "../Modules/EditorGuiModule/EditorGUIModule.h"
#include "../Modules/ProjectModule/ProjectModule.h"

void Editor::initMinor(ModuleManager* moduleManager)
{
	moduleManager->createModule<ProjectModule::ProjectModule>("ProjectModule");
}

void Editor::initMajor(ModuleManager* moduleManager)
{
	m_editorGUIModule = moduleManager->createModule<EditorGUIModule>("EditorGUIModule");
}

void Editor::tick(float deltaTime)
{
	m_editorGUIModule->render();	
}

void Editor::shutdown()
{

}
