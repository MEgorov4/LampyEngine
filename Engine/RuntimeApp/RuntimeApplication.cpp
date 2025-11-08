#include "RuntimeApplication.h"

#include <Modules/ProjectModule/ProjectModule.h>

#include <Foundation/JobSystem/JobSystem.h>

void RuntimeApplication::startup()
{
    Application::run();
}

void RuntimeApplication::onStartupMinor(ContextLocator *locator)
{
    locator->registerMinor(std::make_shared<ProjectModule::ProjectModule>(), 0);
    locator->registerMinor(std::make_shared<EngineCore::Foundation::JobSystem>(), 1);
    locator->startupMinor();
}

void RuntimeApplication::onStartupMajor(ContextLocator *locator)
{
    locator->startupMajor();
}

void RuntimeApplication::onShutdown()
{
    LT_LOGI("Runtime", "shutdown");
}

void RuntimeApplication::render()
{
    // runtime application relies on engine context rendering
}

void RuntimeApplication::tick(float /*deltaTime*/)
{
    // runtime application does not add custom ticking
}

