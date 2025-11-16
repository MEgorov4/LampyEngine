#include "RenderModule.h"

#include "OpenGL/OpenGLRenderer.h"
#include "RenderFactory.h"
#include "RenderLocator.h"
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/WindowModule/WindowModule.h>
#include "RenderConfig.h"
#include <cstdlib>
#include <string_view>

namespace RenderModule
{
void RenderModule::startup()
{
    LT_LOGI("RenderModule", "Startup");

    LT_LOGI("RenderModule", "Create RenderLocator");
    m_context = std::make_unique<RenderContext>();
    LT_ASSERT_MSG(m_context, "Failed to create RenderContext");
    RenderLocator::Provide(m_context.get());

    LT_LOGI("RenderModule", "Create renderer");
    m_renderer = std::make_unique<OpenGL::OpenGLRenderer>();
    LT_ASSERT_MSG(m_renderer, "Failed to create OpenGLRenderer");

    RenderConfig::getInstance().setDebugPassEnabled(m_configuredDebugPassEnabled.value_or(true));
    RenderConfig::getInstance().setGridPassEnabled(m_configuredGridPassEnabled.value_or(true));
    m_renderer->postInit();

    LT_LOGI("RenderModule", "Init render factory");
    RenderFactory::init();

    if (m_configuredOutputMode)
    {
        RenderConfig::getInstance().setOutputMode(*m_configuredOutputMode);
    }
    else if (const char *outputMode = std::getenv("LAMPY_RENDER_OUTPUT"))
    {
        std::string_view mode(outputMode);
        if (mode == "window")
        {
            RenderConfig::getInstance().setOutputMode(RenderOutputMode::WindowSwapchain);
        }
        else if (mode == "offscreen")
        {
            RenderConfig::getInstance().setOutputMode(RenderOutputMode::OffscreenTexture);
        }
    }
}

void RenderModule::applyConfig(const RenderModuleConfig &config)
{
    if (config.outputMode.has_value())
    {
        m_configuredOutputMode = config.outputMode;
    }
    if (config.debugPassEnabled.has_value())
    {
        m_configuredDebugPassEnabled = config.debugPassEnabled;
    }
    if (config.gridPassEnabled.has_value())
    {
        m_configuredGridPassEnabled = config.gridPassEnabled;
    }
}

IRenderer *RenderModule::getRenderer()
{
    IRenderer *renderer = m_renderer.get();
    LT_ASSERT_MSG(renderer, "Renderer is null");
    return renderer;
}

void RenderModule::shutdown()
{
    LT_LOGI("RenderModule", "Shutdown");
    
    if (m_context)
    {
        LT_LOGI("RenderModule", "Clear render scene");
        m_context->clear();
    }
    
    LT_LOGI("RenderModule", "Destroy renderer");
    m_renderer.reset();

    LT_LOGI("RenderModule", "Shutdown render factory");
    RenderFactory::shutdown();
    
    LT_LOGI("RenderModule", "Clear RenderLocator");
    RenderLocator::Provide(nullptr);
    
    if (m_context)
    {
        LT_LOGI("RenderModule", "Release ResourceManager reference");
        m_context->releaseResourceManager();
    }
    
    LT_LOGI("RenderModule", "Destroy context");
    m_context.reset();
}
} // namespace RenderModule
