#include "Editor.h"

#include <Modules/EditorGuiModule/EditorGUIModule.h>
#include <Modules/ImGuiModule/ImGUiModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/ResourceModule/Asset/AssetManager.h>
#include <EngineContext/Foundation/JobSystem/JobSystem.h>

void Editor::startupMinor(ContextLocator &locator)
{
    LT_LOG(LogVerbosity::Info, "Editor", "initMinor");
    locator.registerMinor(std::make_shared<ProjectModule::ProjectModule>(), 0);
    locator.registerMinor(std::make_shared<EngineCore::Foundation::JobSystem>(), 1);
    locator.startupMinor();
}

void Editor::startupMajor(ContextLocator &locator)
{
    LT_LOG(LogVerbosity::Info, "Editor", "initMajor");

    locator.registerMajor(std::make_shared<ImGUIModule::ImGUIModule>(), 10);
    locator.registerMajor(std::make_shared<EditorGUIModule>(), 10);
    locator.startupMajor();
}

void Editor::render()
{
    ZoneScopedN("Editor::render");
    auto *gui = GCXM(EditorGUIModule);
    gui->render(16.67777f);
}

void Editor::tick(float deltaTime)
{
    ZoneScopedN("Editor::tick");
}

void Editor::shutdown()
{
    LT_LOG(LogVerbosity::Info, "Editor", "shutdown");
}
