#include "Editor.h"

#include <Modules/EditorGuiModule/EditorGUIModule.h>
#include <Modules/ImGuiModule/ImGUiModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/ResourceModule/Asset/AssetManager.h>

void Editor::startupMinor(ContextLocator &locator)
{
    LT_LOG(LogVerbosity::Debug, "Editor", "initMinor");
    locator.registerMinor(std::make_shared<ProjectModule::ProjectModule>(), 0);
    locator.startupMinor();
}

void Editor::startupMajor(ContextLocator &locator)
{
    LT_LOG(LogVerbosity::Debug, "Editor", "initMajor");

    locator.registerMajor(std::make_shared<ImGUIModule::ImGUIModule>(), 10);
    locator.registerMajor(std::make_shared<EditorGUIModule>(), 10);
    locator.startupMajor();
}

void Editor::tick(float deltaTime)
{
    LT_PROFILE_ZONE("ECSModule::editorTick");
    auto *gui = GCXM(EditorGUIModule);
    gui->render(deltaTime);
}

void Editor::shutdown()
{
    LT_LOG(LogVerbosity::Debug, "Editor", "shutdown");
}
