#pragma once
#include <EngineMinimal.h>

#include "Abstract/RenderObject.h"

namespace WindowModule
{
    class WindowModule;
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

    class IRenderer
    {
        int m_onECSChanged{};

        TextureHandle m_activeTextureHandle;
        std::unique_ptr<RenderPipelineHandler> m_renderPipelineHandler;

        ECSModule::ECSModule* m_ecsModule;
    public:
        /// <summary>
        /// Constructs an IRenderer with no assigned scene.
        /// </summary>
        IRenderer();

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


        void updateRenderList() const;
        void postInit();

        TextureHandle getOutputRenderHandle(int w, int h);
    };
}
