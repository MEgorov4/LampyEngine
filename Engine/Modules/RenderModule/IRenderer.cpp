#include "IRenderer.h"
#include "IRenderer.h"

#include "../ObjectCoreModule/ECS/ECSModule.h"
#include "../ObjectCoreModule/ECS/ECSComponents.h"
#include "../LoggerModule/Logger.h"

#include "Abstract/RenderResourcesFactory.h"
#include "../ResourceModule/ResourceManager.h"
#include "../WindowModule/Window.h"
#include "../WindowModule/WindowModule.h"
#include "Abstract/RenderPipelineStrategy.h"

namespace RenderModule
{
    IRenderer::IRenderer(std::shared_ptr<Logger::Logger> logger,
                         std::shared_ptr<ResourceModule::ResourceManager> resourceManager,
                         std::shared_ptr<ECSModule::ECSModule> ecsModule,
                         std::shared_ptr<WindowModule::WindowModule> windowModule) : m_logger(logger),
        m_resourceManager(resourceManager),
        m_ecsModule(ecsModule),
        m_windowModule(windowModule)
    {
    }

    IRenderer::~IRenderer()
    {
        m_renderPipelineHandler->cleanup();
        m_ecsModule->OnComponentsChanged.unsubscribe(m_onECSChanged);
    }

    void IRenderer::render()
    {
        m_activeTextureHandle = m_renderPipelineHandler->execute();
        m_windowModule->getWindow()->swapWindow();
    }

    void IRenderer::updateRenderList() const
    {
        m_renderPipelineHandler->parseWorld(m_ecsModule->getCurrentWorld());
    }

    void IRenderer::postInit()
    {
        m_renderPipelineHandler = std::make_unique<RenderPipelineHandler>(m_resourceManager, m_logger);

        m_onECSChanged = m_ecsModule->OnComponentsChanged.
                                      subscribe(std::bind_front(&IRenderer::updateRenderList, this));
    }
}
