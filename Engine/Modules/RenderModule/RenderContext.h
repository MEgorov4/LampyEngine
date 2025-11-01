#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <memory>
#include "Abstract/RenderObject.h"
#include "../ObjectCoreModule/ECS/Components/ECSComponents.h"
#include "../ResourceModule/ResourceManager.h"

namespace RenderModule
{
    /// Камера для шейдеров
    struct CameraRenderData
    {
        glm::mat4 view{1.f};
        glm::mat4 projection{1.f};
        glm::vec4 position{0.f};
    };

    /// Источник света
    struct DirectionalLightRenderData
    {
        glm::vec4 direction{0.f, -1.f, 0.f, 0.f};
        glm::vec4 color{1.f};
        float intensity = 1.f;
    };

    struct PointLightRenderData
    {
        glm::vec4 position{0.f};
        glm::vec4 color{1.f};
        float intensity = 1.f;
    };

    /// Собранная сцена для GPU
    struct RenderScene
    {
        std::vector<RenderObject> objects;
        CameraRenderData camera;
        DirectionalLightRenderData sun;
        std::vector<PointLightRenderData> pointLights;

        void clear()
        {
            objects.clear();
            pointLights.clear();
        }
    };

    /// Контекст, используемый RenderGraph
    class RenderContext
    {
        std::shared_ptr<ResourceModule::ResourceManager> m_resourceManager;
        RenderScene m_scene;
        std::pair<int, int> m_viewportSize{1920, 1080};

    public:
        RenderContext()
            : m_resourceManager(GCM(ResourceModule::ResourceManager)) {}

        void setViewport(int w, int h) { m_viewportSize = {w, h}; }

        const std::pair<int, int>& getViewport() const { return m_viewportSize; }

        RenderScene& scene() { return m_scene; }
        const RenderScene& scene() const { return m_scene; }

        ResourceModule::ResourceManager* resources() const { return m_resourceManager.get(); }
    };
}
