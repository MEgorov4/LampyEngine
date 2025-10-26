#include "Editor.h"

#include <EditorGuiModule/EditorGUIModule.h>
#include <ImGuiModule/ImGUiModule.h>
#include <ProjectModule/ProjectModule.h>

void Editor::initMinor(ContextLocator& locator)
{
    LT_LOG(LogVerbosity::Debug, "Editor", "initMinor");
    locator.registerMinor(std::make_shared<ProjectModule::ProjectModule>(), 0);
    locator.startupMinor();
}

void Editor::initMajor(ContextLocator& locator)
{
    LT_LOG(LogVerbosity::Debug, "Editor", "initMajor");

    locator.registerMajor(std::make_shared<ImGUIModule::ImGUIModule>(), 10);
    locator.registerMinor(std::make_shared<EditorGUIModule>(), 20);
    locator.startupMajor();
}


void Editor::tick(float deltaTime)
{
    auto& gui = Context::Get<EditorGUIModule>();
    gui.render(deltaTime);
}

void Editor::shutdown()
{
    LT_LOG(LogVerbosity::Debug, "Editor", "shutdown");
}
