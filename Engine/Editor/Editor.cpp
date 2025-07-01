#include "Editor.h"

#include "../Modules/ImGuiModule/ImGuiModule.h"
#include "../Modules/EditorGuiModule/EditorGUIModule.h"
#include "../Modules/ProjectModule/ProjectModule.h"

void Editor::initMinor(ModuleManager* moduleManager)
{
	moduleManager->createModule<ProjectModule::ProjectModule>("ProjectModule");
}

void Editor::initMajor(ModuleManager* moduleManager)
{
	m_imGUIModule = moduleManager->createModule<ImGUIModule::ImGUIModule>("ImGuiModule");
	m_editorGUIModule = moduleManager->createModule<EditorGUIModule>("EditorGUIModule");
}

void Editor::tick(float deltaTime)
{
	m_editorGUIModule->render(deltaTime);	
}

void Editor::shutdown()
{

}
