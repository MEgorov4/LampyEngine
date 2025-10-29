#include "IRenderer.h"

#include "Abstract/RenderPipelineStrategy.h"
#include "Abstract/RenderResourcesFactory.h"

#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/WindowModule/Window.h>
#include <Modules/WindowModule/WindowModule.h>

namespace RenderModule
{

IRenderer::IRenderer() : m_ecsModule(GCM(ECSModule::ECSModule))
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
}

TextureHandle IRenderer::getOutputRenderHandle(int w, int h)
{
    m_renderPipelineHandler->resize(w, h);
    return m_activeTextureHandle;
}

void IRenderer::updateRenderList() const
{
    m_renderPipelineHandler->cleanup();
    m_renderPipelineHandler->parseWorld(m_ecsModule->getCurrentWorld());
}

void IRenderer::postInit()
{
    m_renderPipelineHandler = std::make_unique<RenderPipelineHandler>();

    m_onECSChanged = m_ecsModule->OnComponentsChanged.subscribe(std::bind_front(&IRenderer::updateRenderList, this));
}
} // namespace RenderModule
