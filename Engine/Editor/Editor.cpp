#include "Editor.h"
#include "../Modules/ImGuiModule/ImGuiModule.h"
void Editor::init()
{
	ImGuiModule::getInstance().startUp();
}

void Editor::tick(float deltaTime)
{
	ImGuiModule::getInstance().renderUI();
}

void Editor::shutDown()
{
	ImGuiModule::getInstance().shutDown();
}
