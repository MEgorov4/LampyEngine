#include "Editor.h"
#include "../Modules/EditorGuiModule/EditorGUIModule.h"
#include "../Modules/ProjectModule/ProjectModule.h"

void Editor::initMinor()
{
	LOG_INFO("Editor: Init");
	LOG_INFO("Editor: startup editor modules");
	ProjectModule::getInstance().startup();
}

void Editor::initMajor()
{
	startupEditorModules();
}


void Editor::startupEditorModules()
{
	EditorGUIModule::getInstance().startup();
}

void Editor::tick(float deltaTime)
{
	EditorGUIModule::getInstance().render();
}

void Editor::shutDown()
{
	LOG_INFO("Editor: Shut down editor");
	shutDownEditorModules();
}

void Editor::shutDownEditorModules()
{
	LOG_INFO("Editor: Shut down editor modules");
	EditorGUIModule::getInstance().shutDown();
	ProjectModule::getInstance().shutDown();
}
