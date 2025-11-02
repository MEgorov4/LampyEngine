#include "RenderModule.h"

#include "OpenGL/OpenGLRenderer.h"
#include "RenderFactory.h"
#include "RenderLocator.h"
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/WindowModule/WindowModule.h>

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
    m_renderer->postInit();

    LT_LOGI("RenderModule", "Init render factory");
    RenderFactory::init();
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
