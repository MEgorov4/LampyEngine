#include "RuntimeApplication.h"

#include <Modules/RenderModule/RenderModule.h>
#include <Modules/ProjectModule/ProjectModule.h>

void RuntimeApplication::startup()
{
    Application::run();
}

void RuntimeApplication::setProjectFileOverride(const std::string &path)
{
    if (!path.empty())
    {
        m_projectFileOverride = path;
    }
}

void RuntimeApplication::onStartupMinor(ContextLocator *locator)
{
    auto projectModule = std::make_shared<ProjectModule::ProjectModule>();
    if (m_projectFileOverride)
    {
        projectModule->setProjectFileOverride(*m_projectFileOverride);
    }
    locator->registerMinor(projectModule, 0);
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

void RuntimeApplication::configureModules(ModuleConfigRegistry &registry)
{
    RenderModule::RenderModuleConfig renderCfg;
    renderCfg.outputMode = RenderModule::RenderOutputMode::WindowSwapchain;
    registry.setConfig<RenderModule::RenderModule>(renderCfg);
}

