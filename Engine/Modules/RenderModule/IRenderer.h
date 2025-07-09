#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "Abstract/RenderObject.h"

namespace WindowModule
{
    class WindowModule;
}

namespace Logger
{
    class Logger;
}

namespace ECSModule
{
    class ECSModule;
}

namespace ResourceModule
{
    class ResourceManager;
}

namespace RenderModule
{
    class RenderPipelineHandler;
    class IShader;
    class ITexture;

    class IRenderer
    {
        int m_onECSChanged{};

        TextureHandle m_activeTextureHandle;
        std::unique_ptr<RenderPipelineHandler> m_renderPipelineHandler;

    protected:
        std::shared_ptr<ResourceModule::ResourceManager> m_resourceManager;
        std::shared_ptr<ECSModule::ECSModule> m_ecsModule;
        std::shared_ptr<Logger::Logger> m_logger;
        std::shared_ptr<WindowModule::WindowModule> m_windowModule;

        RenderPipelineData m_activeRenderPipelineData{};
        RenderPipelineData m_updateRenderPipelineData{};

        std::shared_ptr<ITexture> m_albedoGeneric;
        std::shared_ptr<ITexture> m_emissionGeneric;
        std::shared_ptr<ITexture> m_emptyTextureGeneric;

        std::shared_ptr<IShader> m_debugLineShader;

    public:
        /// <summary>
        /// Constructs an IRenderer with no assigned scene.
        /// </summary>
        IRenderer(std::shared_ptr<Logger::Logger> logger,
                  std::shared_ptr<ResourceModule::ResourceManager> resourceManager,
                  std::shared_ptr<ECSModule::ECSModule> ecsModule,
                  std::shared_ptr<WindowModule::WindowModule> windowModule);

        /// <summary>
        /// Virtual destructor to ensure proper cleanup of derived renderers.
        /// </summary>
        virtual ~IRenderer();

        /// <summary>
        /// Renders a single frame.
        /// </summary>
        void render();

        /// <summary>
        /// Waits for the renderer to complete all rendering operations before proceeding.
        /// </summary>
        virtual void waitIdle() = 0;

        virtual void drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color)
        {
        };

        virtual TextureHandle getOutputRenderHandle() { return m_activeTextureHandle; }

        void updateRenderList() const;
        void postInit();
    };
}
