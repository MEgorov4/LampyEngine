#include "Editor.h"
#include "../Modules/EditorGuiModule/EditorGUIModule.h"
#include "../Modules/ProjectModule/ProjectModule.h"

void Editor::init()
{
	LOG_INFO("Editor: Init");
	LOG_INFO("Editor: startup editor modules");
	startupEditorModules();
}


void Editor::startupEditorModules()
{
	ProjectModule::getInstance().startUp();
	EditorGUIModule::getInstance().startUp();
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
