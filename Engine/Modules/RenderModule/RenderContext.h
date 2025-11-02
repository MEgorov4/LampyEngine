#pragma once
#include "../../EngineContext/Core/Core.h"
#include "../../EngineContext/Foundation/Assert/Assert.h"
#include "../ObjectCoreModule/ECS/Components/ECSComponents.h"
#include "../ResourceModule/ResourceManager.h"
#include "Abstract/RenderObject.h"
#include "Foundation/Profiler/ProfileAllocator.h"

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
// OpenGL headers must be included before TracyOpenGL.hpp
// Only include OpenGL when available (GPU profiling)
#if defined(__has_include) && __has_include(<GL/glew.h>)
#include <GL/glew.h>
#include <tracy/TracyOpenGL.hpp>
#define RENDER_CONTEXT_HAS_OPENGL 1
#else
#define RENDER_CONTEXT_HAS_OPENGL 0
#endif
#else
#define RENDER_CONTEXT_HAS_OPENGL 0
#endif

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
    glm::mat4 lightViewMatrix{1.f};
    glm::mat4 lightProjectionMatrix{1.f};
};

struct PointLightRenderData
{
    glm::vec4 position{0.f};
    glm::vec4 color{1.f};
    float intensity = 1.f;
    float innerRadius = 0.0f;  // Внутренний радиус (полная интенсивность)
    float outerRadius = 10.0f; // Внешний радиус (затухание до 0)
};

/// Отладочный примитив - линия
struct DebugLine
{
    glm::vec3 from;
    glm::vec3 to;
    glm::vec3 color;
};

/// Отладочный примитив - бокс (AABB)
struct DebugBox
{
    glm::vec3 center;
    glm::vec3 size;
    glm::vec3 color;
};

/// Отладочный примитив - сфера
struct DebugSphere
{
    glm::vec3 center;
    float radius;
    glm::vec3 color;
};

/// Собранная сцена для GPU
struct RenderScene
{
    std::vector<RenderObject, ProfileAllocator<RenderObject>> objects;
    CameraRenderData camera;
    DirectionalLightRenderData sun;
    std::vector<PointLightRenderData, ProfileAllocator<PointLightRenderData>> pointLights;

    // Отладочные примитивы
    std::vector<DebugLine, ProfileAllocator<DebugLine>> debugLines;
    std::vector<DebugBox, ProfileAllocator<DebugBox>> debugBoxes;
    std::vector<DebugSphere, ProfileAllocator<DebugSphere>> debugSpheres;

    void clear()
    {
        objects.clear();
        pointLights.clear();
        // Отладочные примитивы НЕ очищаем здесь - они управляются через flushDebugPrimitives()
        // и очищаются в начале каждого кадра перед переносом из буфера ожидания
    }
};

/// Контекст, используемый RenderGraph
class RenderContext
{
    std::shared_ptr<ResourceModule::ResourceManager> m_resourceManager;
    RenderScene m_scene;
    std::pair<int, int> m_viewportSize{1920, 1080};

    // Двухбуферная система для отладочных примитивов
    // Эти буферы можно заполнять из любого потока в любое время
    std::vector<DebugLine, ProfileAllocator<DebugLine>> m_pendingDebugLines;
    std::vector<DebugBox, ProfileAllocator<DebugBox>> m_pendingDebugBoxes;
    std::vector<DebugSphere, ProfileAllocator<DebugSphere>> m_pendingDebugSpheres;

    // Мьютекс для потокобезопасности (если нужно будет использовать из разных потоков)
    // std::mutex m_debugMutex;

#ifdef TRACY_ENABLE
#if RENDER_CONTEXT_HAS_OPENGL
    bool m_gpuCtxInit = false;
    bool m_gpuSupported = false;
#endif
#endif

  public:
    RenderContext()
    {
        // Получаем shared_ptr напрямую из CoreLocator, чтобы не создавать отдельный счётчик ссылок
        auto shared = Core::Locator().tryGet<ResourceModule::ResourceManager>();
        LT_ASSERT_MSG(shared, "ResourceManager not found in CoreLocator");
        m_resourceManager = shared;
    }

    ~RenderContext()
    {
#ifdef TRACY_ENABLE
#if RENDER_CONTEXT_HAS_OPENGL
        if (m_gpuCtxInit)
        {
            // Tracy GPU context is destroyed automatically when GL context is destroyed
            // No explicit cleanup needed
            m_gpuCtxInit = false;
        }
#endif
#endif
    }

    // Call this AFTER glewInit() and when GL context is current
    void initAfterGL()
    {
#ifdef TRACY_ENABLE
#if RENDER_CONTEXT_HAS_OPENGL
        // Check for timer query extension (required for Tracy OpenGL)
        m_gpuSupported = (GLEW_ARB_timer_query != 0);
        
        if (!m_gpuSupported)
        {
            LT_LOGW("TracyGPU", "GL_ARB_timer_query not supported. Disabling GPU zones.");
            return;
        }

        // Create Tracy GPU context on current GL context
        // TracyGpuContext is a macro that initializes the context and returns pointer
        TracyGpuContext;
        m_gpuCtxInit = true;
        
        LT_LOGI("TracyGPU", "Tracy GPU context initialized.");
#endif
#endif
    }

    bool isGpuProfilerReady() const noexcept
    {
#ifdef TRACY_ENABLE
#if RENDER_CONTEXT_HAS_OPENGL
        return m_gpuCtxInit && m_gpuSupported;
#else
        return false;
#endif
#else
        return false;
#endif
    }

    void beginFrame()
    {
#ifdef TRACY_ENABLE
        ZoneScopedN("RenderContext::beginFrame");
#endif
    }

    void endFrame()
    {
#ifdef TRACY_ENABLE
        ZoneScopedN("RenderContext::endFrame");
#if RENDER_CONTEXT_HAS_OPENGL
        if (m_gpuCtxInit)
        {
            TracyGpuCollect;
        }
#endif
#endif
    }

    void setViewport(int w, int h)
    {
        LT_ASSERT_MSG(w > 0 && h > 0, "Viewport dimensions must be positive");
        m_viewportSize = {w, h};
    }

    const std::pair<int, int> &getViewport() const
    {
        return m_viewportSize;
    }

    RenderScene &scene()
    {
        return m_scene;
    }
    const RenderScene &scene() const
    {
        return m_scene;
    }

    ResourceModule::ResourceManager *resources() const
    {
        LT_ASSERT_MSG(m_resourceManager, "ResourceManager is null");
        return m_resourceManager.get();
    }

    void clear()
    {
        m_scene.clear();
    }

    // Методы для добавления отладочных примитивов (можно вызывать из любого места)
    void addDebugLine(const DebugLine &line)
    {
        m_pendingDebugLines.push_back(line);
    }

    void addDebugBox(const DebugBox &box)
    {
        m_pendingDebugBoxes.push_back(box);
    }

    void addDebugSphere(const DebugSphere &sphere)
    {
        m_pendingDebugSpheres.push_back(sphere);
    }

    // Переносит отладочные примитивы из буфера ожидания в сцену
    // Вызывается в начале render() перед использованием
    void flushDebugPrimitives()
    {
        // Переносим примитивы из буферов ожидания в сцену
        m_scene.debugLines.clear();
        m_scene.debugLines.insert(m_scene.debugLines.end(), m_pendingDebugLines.begin(), m_pendingDebugLines.end());

        m_scene.debugBoxes.clear();
        m_scene.debugBoxes.insert(m_scene.debugBoxes.end(), m_pendingDebugBoxes.begin(), m_pendingDebugBoxes.end());

        m_scene.debugSpheres.clear();
        m_scene.debugSpheres.insert(m_scene.debugSpheres.end(), m_pendingDebugSpheres.begin(),
                                    m_pendingDebugSpheres.end());

        // Очищаем буферы ожидания для следующего кадра
        m_pendingDebugLines.clear();
        m_pendingDebugBoxes.clear();
        m_pendingDebugSpheres.clear();
    }

    // Явное освобождение shared_ptr на ResourceManager перед уничтожением
    void releaseResourceManager()
    {
        m_resourceManager.reset();
    }
};
} // namespace RenderModule
