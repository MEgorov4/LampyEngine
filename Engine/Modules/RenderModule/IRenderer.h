#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "Abstract/RenderObject.h"

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
    class IShader;
    class ITexture;

    class IRenderer
    {
        std::shared_ptr<ResourceModule::ResourceManager> m_resourceManager;
        std::shared_ptr<ECSModule::ECSModule> m_ecsModule;
        
        int m_onECSChanged;
        std::shared_ptr<IShader> m_shadowsShader;
        std::shared_ptr<IShader> m_reflectionsShader;
        std::shared_ptr<IShader> m_lightsShader;
        std::shared_ptr<IShader> m_finalShader;
        std::shared_ptr<IShader> m_textureShader;
        
    protected:
        RenderPipelineData m_activeRenderPipelineData;
        RenderPipelineData m_updateRenderPipelineData;

        std::shared_ptr<ITexture> m_albedoGeneric;
        std::shared_ptr<ITexture> m_emissionGeneric;
        std::shared_ptr<ITexture> m_emptyTextureGeneric;

        std::shared_ptr<IShader> m_debugLineShader;

    public:
        /// <summary>
        /// Constructs an IRenderer with no assigned scene.
        /// </summary>
        IRenderer(std::shared_ptr<ResourceModule::ResourceManager> resourceManager, std::shared_ptr<ECSModule::ECSModule> ecsModule);

        /// <summary>
        /// Virtual destructor to ensure proper cleanup of derived renderers.
        /// </summary>
        virtual ~IRenderer();

        /// <summary>
        /// Renders a single frame.
        /// </summary>
        virtual void render() = 0;

        virtual void* getOffscreenImageDescriptor() = 0;
        /// <summary>
        /// Waits for the renderer to complete all rendering operations before proceeding.
        /// </summary>
        virtual void waitIdle() = 0;

        // DEBUG FUNCTION
        virtual void drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) = 0;

        void updateRenderList();
        void postInit();
    };
}
