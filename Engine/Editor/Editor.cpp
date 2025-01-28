#include "Editor.h"

#include "../Modules/EditorGuiModule/EditorGUIModule.h"

void Editor::init()
{
	startupEditorModules();
}

void Editor::startupEditorModules()
{

	EditorGUIModule::getInstance().startUp();
}

void Editor::tick(float deltaTime)
{
	EditorGUIModule::getInstance().render();
}

void Editor::shutDown()
{
	shutDownEditorModules();
}

void Editor::shutDownEditorModules()
{
	EditorGUIModule::getInstance().shutDown();
}
