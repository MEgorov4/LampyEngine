#pragma once
#include "../ObjectCoreModule/ECS/Components/ECSComponents.h"
#include "../ResourceModule/ResourceManager.h"
#include "Abstract/RenderObject.h"
#include "Foundation/Profiler/ProfileAllocator.h"
#include "../../EngineContext/Core/Core.h"
#include "../../EngineContext/Foundation/Assert/Assert.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <stdexcept>
#include <vector>

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
    std::vector<RenderObject, ProfileAllocator<RenderObject>> objects;
    CameraRenderData camera;
    DirectionalLightRenderData sun;
    std::vector<PointLightRenderData, ProfileAllocator<PointLightRenderData>> pointLights;

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
    {
        // Получаем shared_ptr напрямую из CoreLocator, чтобы не создавать отдельный счётчик ссылок
        auto shared = Core::Locator().tryGet<ResourceModule::ResourceManager>();
        LT_ASSERT_MSG(shared, "ResourceManager not found in CoreLocator");
        m_resourceManager = shared;
    }

    void setViewport(int w, int h)
    {
        LT_ASSERT_MSG(w > 0 && h > 0, "Viewport dimensions must be positive");
        m_viewportSize = {w, h};
    }

    const std::pair<int, int>& getViewport() const
    {
        return m_viewportSize;
    }

    RenderScene& scene()
    {
        return m_scene;
    }
    const RenderScene& scene() const
    {
        return m_scene;
    }

    ResourceModule::ResourceManager* resources() const
    {
        LT_ASSERT_MSG(m_resourceManager, "ResourceManager is null");
        return m_resourceManager.get();
    }

    void clear()
    {
        m_scene.clear();
    }

    // Явное освобождение shared_ptr на ResourceManager перед уничтожением
    void releaseResourceManager()
    {
        m_resourceManager.reset();
    }
};
} // namespace RenderModule
