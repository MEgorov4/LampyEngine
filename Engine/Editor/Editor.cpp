#include "Editor.h"

#include <Modules/ImGuiModule/ImGUiModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Editor/Modules/EditorGuiModule/EditorGUIModule.h>
#include <Modules/ResourceModule/Asset/AssetManager.h>

void EditorApplication::startup()
{
    Application::run();
}


void EditorApplication::onStartupMinor(ContextLocator* locator)
{
    LT_LOG(LogVerbosity::Info, "Editor", "initMinor");
    locator->registerMinor(std::make_shared<ProjectModule::ProjectModule>(), 0);
    locator->startupMinor();
}


void EditorApplication::onStartupMajor(ContextLocator* locator)
{
    LT_LOG(LogVerbosity::Info, "Editor", "initMajor");
    locator->registerMajor(std::make_shared<EditorGUIModule>(), 10);
    locator->startupMajor();
}


void EditorApplication::onShutdown()
{
    LT_LOGI("Editor", "shutdown");
}

void EditorApplication::render()
{
    ZoneScopedN("Editor::render");
    auto *gui = GCXM(EditorGUIModule);
    gui->render(16.67777f);
}

void EditorApplication::tick(float deltaTime)
{
    ZoneScopedN("Editor::tick");
}

