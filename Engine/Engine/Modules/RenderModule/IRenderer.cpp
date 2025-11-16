#include "IRenderer.h"

#include <EngineMinimal.h>

#include "Foundation/Assert/Assert.h"
#include "RenderContext.h"
#include "RenderGraph/RenderGraphBuilder.h"
#include "RenderGraph/RenderNodes.h"
#include "RenderLocator.h"
#include "RenderObjectFactory.h"
#include "UIRenderPass.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Core/EventHelpers.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/PhysicsModule/PhysicsLocator.h>
#include <Modules/PhysicsModule/PhysicsContext/PhysicsContext.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/WindowModule/WindowModule.h>
#include <Modules/WindowModule/Window.h>

namespace RenderModule
{
IRenderer::IRenderer()
    : m_ecsModule(GCM(ECSModule::ECSModule)), m_listManager(m_entityTracker),
      m_transformUpdater(m_entityTracker, m_listManager)
{
    LT_ASSERT_MSG(m_ecsModule, "Failed to get ECSModule from CoreLocator");
}

IRenderer::~IRenderer()
{
}

void IRenderer::postInit()
{
    auto *ctxPtr = RenderLocator::Get();
    if (ctxPtr)
    {
        m_listManager.setContext(ctxPtr);
        m_cameraUpdater = std::make_unique<CameraUpdater>(ctxPtr);
    }

    const bool debugPassEnabled = RenderConfig::getInstance().getDebugPassEnabled();
    const bool gridPassEnabled = RenderConfig::getInstance().getGridPassEnabled();

    RenderGraphBuilder builder(m_renderGraph);

    builder.addResource("shadow_pass_depth", 1920, 1080)
        .addResource("texture_pass_color", 1920, 1080);

    if (gridPassEnabled)
    {
        builder.addResource("grid_pass_color", 1920, 1080);
    }

    if (debugPassEnabled)
    {
        builder.addResource("debug_pass_color", 1920, 1080);
    }

    builder.addResource("final", 1920, 1080);

    builder.addPass("Shadow")
        .write("shadow_pass_depth")
        .exec(RenderNodes::ShadowPass)
        .end();

    builder.addPass("PBR")
        .read("shadow_pass_depth")
        .write("texture_pass_color")
        .exec(RenderNodes::PBRPass)
        .end();

    const char *finalInputResource = "texture_pass_color";
    if (gridPassEnabled)
    {
        builder.addPass("Grid")
            .read("texture_pass_color")
            .write("grid_pass_color")
            .exec(RenderNodes::GridPass)
            .end();

        finalInputResource = "grid_pass_color";
    }

    if (debugPassEnabled)
    {
        builder.addPass("Debug")
            .read(finalInputResource)
            .write("debug_pass_color")
            .exec(RenderNodes::DebugPass)
            .end();

        finalInputResource = "debug_pass_color";
    }

    builder.addPass("Final")
        .read(finalInputResource)
        .write("final")
        .exec(RenderNodes::FinalCompose)
        .end()

        .build();

    setupEventSubscriptions();

    setupRenderObservers();
}

void IRenderer::setupEventSubscriptions()
{
    using namespace Events::ECS;

    m_eventBinder.bind<WorldOpened>([this](const WorldOpened &event) { onWorldOpened(event); });

    m_eventBinder.bind<WorldClosed>([this](const WorldClosed &event) { onWorldClosed(event); });

    m_eventBinder.bind<EntityCreated>([this](const EntityCreated &event) { onEntityCreated(event); });

    m_eventBinder.bind<EntityDestroyed>([this](const EntityDestroyed &event) { onEntityDestroyed(event); });

    m_eventBinder.bind<ComponentChanged>([this](const ComponentChanged &event) { onComponentChanged(event); });

    m_eventBinder.bind<RenderFrameData>([this](const RenderFrameData &event) { onRenderFrameData(event); });
}

void IRenderer::setupRenderObservers()
{
    auto *worldPtr = m_ecsModule->getCurrentWorld();
    if (!worldPtr)
        return;

    auto &world = worldPtr->get();
    m_renderObserver.initialize(world, m_entityTracker);

    LT_LOGI("Renderer", "Render observers initialized");
}

void IRenderer::onWorldOpened(const Events::ECS::WorldOpened &event)
{
    LT_LOGI("Renderer", "World opened: " + event.name + " - rebuilding render list");
    m_needsFullRebuild = true;
    m_entityTracker.clear();

    auto *worldPtr = m_ecsModule->getCurrentWorld();
    if (worldPtr)
    {
        auto &world = worldPtr->get();
        m_renderObserver.initialize(world, m_entityTracker);
    }
}

void IRenderer::onWorldClosed(const Events::ECS::WorldClosed &event)
{
    LT_LOGI("Renderer", "World closed: " + event.name + " - clearing render list");
    m_entityTracker.clear();
    m_listManager.clear();
    m_needsFullRebuild = true;
}

void IRenderer::onEntityCreated(const Events::ECS::EntityCreated &event)
{
    LT_LOGI("Renderer", "Entity created: " + event.name + " (id: " + std::to_string(event.id) + ")");
}

void IRenderer::onEntityDestroyed(const Events::ECS::EntityDestroyed &event)
{
    m_listManager.removeObject(event.id);
    m_entityTracker.removeState(event.id);

    LT_LOGI("Renderer", "Entity destroyed (id: " + std::to_string(event.id) + ") - removed from render list");
}

void IRenderer::onComponentChanged(const Events::ECS::ComponentChanged &event)
{
    if (event.componentName == "MeshComponent")
    {
        auto *worldPtr = m_ecsModule->getCurrentWorld();
        if (!worldPtr)
            return;

        auto &world = worldPtr->get();
        auto entity = world.entity(event.entityId);
        if (!entity.is_valid())
            return;

        const MeshComponent *mesh = entity.get<MeshComponent>();
        if (!mesh)
            return;

        bool hasTransform = entity.has<TransformComponent>();
        bool hasMesh = entity.has<MeshComponent>();

        LT_LOGI("Renderer", "ComponentChanged: MeshComponent for entity " + std::to_string(event.entityId) +
                                " - transform:" + std::to_string(hasTransform) +
                                " mesh:" + std::to_string(hasMesh));

        auto *state = m_entityTracker.getState(event.entityId);
        if (state)
        {
            if (state->hasMeshChanged(*mesh))
            {
                RenderDiff diff;
                RenderDiff::EntityChange change;
                change.type = RenderDiff::EntityChange::Updated;
                change.entityId = event.entityId;
                change.newState = std::make_unique<EntityRenderState>();

                const TransformComponent *transform = entity.get<TransformComponent>();

                if (transform)
                {
                    change.newState->position = transform->position;
                    change.newState->rotation = transform->rotation;
                    change.newState->scale = transform->scale;
                }
                change.newState->mesh = *mesh;
                const MaterialComponent* material = entity.get<MaterialComponent>();
                if (material)
                    change.newState->material = *material;
                change.newState->entityId = event.entityId;
                change.newState->isValid = true;

                diff.changes.push_back(std::move(change));

                applyRenderDiff(diff);

                LT_LOGI("Renderer",
                        "ComponentChanged: MeshComponent updated for entity " + std::to_string(event.entityId));
            }
        }
        else if (hasTransform && hasMesh)
        {
            RenderDiff diff;
            RenderDiff::EntityChange change;
            change.type = RenderDiff::EntityChange::Added;
            change.entityId = event.entityId;
            change.newState = std::make_unique<EntityRenderState>();

            const TransformComponent *transform = entity.get<TransformComponent>();

            if (transform)
            {
                change.newState->position = transform->position;
                change.newState->rotation = transform->rotation;
                change.newState->scale = transform->scale;
            }
            change.newState->mesh = *mesh;
            const MaterialComponent* material = entity.get<MaterialComponent>();
            if (material)
                change.newState->material = *material;
            change.newState->entityId = event.entityId;
            change.newState->isValid = true;

            diff.changes.push_back(std::move(change));

            applyRenderDiff(diff);

            LT_LOGI("Renderer", "ComponentChanged: MeshComponent added for entity " + std::to_string(event.entityId) +
                                    " (observer fallback - all components present)");
        }
        else
        {
            LT_LOGI("Renderer", "ComponentChanged: MeshComponent for entity " + std::to_string(event.entityId) +
                                    " - waiting for missing components");
        }
    }
}

void IRenderer::onRenderFrameData(const Events::ECS::RenderFrameData &event)
{

    if (m_cameraUpdater)
    {
        m_cameraUpdater->updateFromEvent(event.camera);
    }

    m_transformUpdater.updateFromEvent(event);
}

void IRenderer::updateRenderList()
{
    ZoneScopedN("IRenderer::updateRenderList");

    if (m_needsFullRebuild)
    {
        m_needsFullRebuild = false;
        rebuildRenderList();
        return;
    }

    RenderDiff diff = m_renderObserver.consumeDiff();

    if (!diff.empty())
    {
        applyRenderDiff(diff);
    }

    // ============================================================
    // ============================================================
    //{
    //    if (world.component<ECSModule::DirectionalLightComponent>().is_valid())
    //    {
    //        auto qDirLight = world.query<ECSModule::DirectionalLightComponent>();
    //        qDirLight.each(
    //            [&](flecs::entity, const ECSModule::DirectionalLightComponent& light)
    //            {
    //                scene.sun.direction = glm::vec4(light.direction, 0.0f);
    //                scene.sun.color     = glm::vec4(light.color, 1.0f);
    //                scene.sun.intensity = light.intensity;
    //            });
    //    }
    //    else
    //    {
    //        scene.sun.direction = glm::vec4(0.f, -1.f, 0.f, 0.f);
    //        scene.sun.color     = glm::vec4(1.f);
    //        scene.sun.intensity = 1.f;
    //    }
    //}

    //// ============================================================
    //// ============================================================
    //{
    //    if (world.component<ECSModule::PointLightComponent>().is_valid())
    //    {
    //        auto qPoint = world.query<ECSModule::PointLightComponent>();
    //        qPoint.each(
    //            [&](flecs::entity, const ECSModule::PointLightComponent& light)
    //            {
    //                scene.pointLights.push_back({
    //                    .position  = glm::vec4(light.position, 1.f),
    //                    .color     = glm::vec4(light.color, 1.f),
    //                    .intensity = light.intensity
    //                });
    //            });
    //    }

    updateLightsFromECS();
}

void IRenderer::render()
{
    ZoneScopedN("IRenderer::render");

    auto *ctxPtr = RenderLocator::Get();
    const auto outputMode = RenderConfig::getInstance().getOutputMode();

    if (ctxPtr)
    {
        ctxPtr->beginFrame();
        
        // Call physics debug draw before flushing debug primitives
        if (auto* physicsCtx = PhysicsModule::PhysicsLocator::TryGet())
        {
            if (physicsCtx->isDebugDrawEnabled())
            {
                physicsCtx->debugDraw();
            }
        }
        
        ctxPtr->flushDebugPrimitives();
    }

    updateRenderList();
    
    if (outputMode == RenderOutputMode::WindowSwapchain)
    {
        if (auto *windowModule = GCM(WindowModule::WindowModule))
        {
            if (auto *window = windowModule->getWindow())
            {
                auto viewport = window->getWindowSize();
                if (viewport.first > 0 && viewport.second > 0)
                {
                    if (ctxPtr)
                    {
                        ctxPtr->setViewport(viewport.first, viewport.second);
                    }
                    m_renderGraph.resizeAll(viewport.first, viewport.second);
                }
            }
        }
    }

    m_activeTextureHandle = m_renderGraph.execute();

    if (outputMode == RenderOutputMode::WindowSwapchain)
    {
        // Present the final composed scene to the main window.
        presentToWindow(m_activeTextureHandle);

        // Render UI on top of the presented scene using the configured UI backend.
        UIRenderPass::render();
    }

    if (ctxPtr)
    {
        ctxPtr->endFrame();
    }
}

void IRenderer::drawWorldGrid()
{
    ZoneScopedN("IRenderer::drawWorldGrid");
    auto *ctxPtr = RenderLocator::Get();
    if (!ctxPtr)
        return;


    const float gridSize = 100.0f; 
    const float gridStep = 1.0f;   
    const float majorStep = 10.0f; 

    
    const glm::vec3 majorLineColor = glm::vec3(0.5f, 0.5f, 0.5f); 
    const glm::vec3 minorLineColor = glm::vec3(0.3f, 0.3f, 0.3f); 
    const glm::vec3 axisColorX = glm::vec3(1.0f, 0.2f, 0.2f);     
    const glm::vec3 axisColorZ = glm::vec3(0.2f, 0.2f, 1.0f);     

    const float y = 0.0f; 

    {
        DebugLine axisX;
        axisX.from = glm::vec3(-gridSize, y, 0.0f);
        axisX.to = glm::vec3(gridSize, y, 0.0f);
        axisX.color = axisColorX;
        ctxPtr->addDebugLine(axisX);

        DebugLine axisZ;
        axisZ.from = glm::vec3(0.0f, y, -gridSize);
        axisZ.to = glm::vec3(0.0f, y, gridSize);
        axisZ.color = axisColorZ;
        ctxPtr->addDebugLine(axisZ);
    }

    for (float x = -gridSize; x <= gridSize; x += gridStep)
    {
        bool isAxis = (std::abs(x) < 0.001f);
        if (isAxis)
            continue; 

        float remainder = std::abs(std::fmod(std::abs(x), majorStep));
        bool isMajor = (remainder < 0.001f || remainder > (majorStep - 0.001f));

        DebugLine line;
        line.from = glm::vec3(x, y, -gridSize);
        line.to = glm::vec3(x, y, gridSize);
        line.color = isMajor ? majorLineColor : minorLineColor;
        ctxPtr->addDebugLine(line);
    }

    for (float z = -gridSize; z <= gridSize; z += gridStep)
    {
        bool isAxis = (std::abs(z) < 0.001f);
        if (isAxis)
            continue; 

        float remainder = std::abs(std::fmod(std::abs(z), majorStep));
        bool isMajor = (remainder < 0.001f || remainder > (majorStep - 0.001f));

        DebugLine line;
        line.from = glm::vec3(-gridSize, y, z);
        line.to = glm::vec3(gridSize, y, z);
        line.color = isMajor ? majorLineColor : minorLineColor;
        ctxPtr->addDebugLine(line);
    }
}

TextureHandle IRenderer::getOutputRenderHandle(int w, int h)
{
    const int minWidth = 1;
    const int minHeight = 1;

    if (w < minWidth || h < minHeight)
    {
        return m_activeTextureHandle;
    }

    LT_ASSERT_MSG(w > 0 && h > 0, "Output dimensions must be positive");

    if (RenderConfig::getInstance().getOutputMode() == RenderOutputMode::OffscreenTexture)
    {
        auto *ctxPtr = RenderLocator::Get();
        if (ctxPtr)
        {
            ctxPtr->setViewport(w, h);
        }

        m_renderGraph.resizeAll(w, h);
    }

    return m_activeTextureHandle;
}

void IRenderer::drawLine(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &color)
{
    auto *ctxPtr = RenderLocator::Get();
    if (!ctxPtr)
        return;

    DebugLine line;
    line.from = from;
    line.to = to;
    line.color = color;
    ctxPtr->addDebugLine(line);
}

void IRenderer::drawBox(const glm::vec3 &center, const glm::vec3 &size, const glm::vec3 &color)
{
    auto *ctxPtr = RenderLocator::Get();
    if (!ctxPtr)
        return;

    DebugBox box;
    box.center = center;
    box.size = size;
    box.color = color;
    ctxPtr->addDebugBox(box);
}

void IRenderer::drawSphere(const glm::vec3 &center, float radius, const glm::vec3 &color)
{
    auto *ctxPtr = RenderLocator::Get();
    if (!ctxPtr)
        return;

    DebugSphere sphere;
    sphere.center = center;
    sphere.radius = radius;
    sphere.color = color;
    ctxPtr->addDebugSphere(sphere);
}

void IRenderer::applyRenderDiff(const RenderDiff &diff)
{
    ZoneScopedN("IRenderer::applyRenderDiff");

    for (const auto &change : diff.changes)
    {
        switch (change.type)
        {
        case RenderDiff::EntityChange::Added: {
            if (!change.newState || !change.newState->isValid)
                break;

            auto &state = m_entityTracker.getOrCreateState(change.entityId);

            state.entityId = change.newState->entityId;
            state.isValid = change.newState->isValid;
            state.position = change.newState->position;
            state.rotation = change.newState->rotation;
            state.scale = change.newState->scale;
            state.mesh = change.newState->mesh;
            state.material = change.newState->material;

            RenderObject obj = RenderObjectFactory::createFromState(state);
            m_listManager.addObject(obj, change.entityId);

            LT_LOGI("Renderer", "Applied diff: Added entity " + std::to_string(change.entityId));
            break;
        }

        case RenderDiff::EntityChange::Updated: {
            if (!change.newState)
                break;

            auto *state = m_entityTracker.getState(change.entityId);
            if (!state)
            {
                LT_LOGE("Renderer",
                        "Cannot update entity " + std::to_string(change.entityId) + " - not found in tracker");
                break;
            }

            state->position = change.newState->position;
            state->rotation = change.newState->rotation;
            state->scale = change.newState->scale;
            state->mesh = change.newState->mesh;
            state->material = change.newState->material;

            size_t *pIndex = m_listManager.getObjectIndex(change.entityId);
            if (!pIndex || !m_listManager.isValidIndex(*pIndex))
            {
                LT_LOGE("Renderer", "Cannot find render object for entity " + std::to_string(change.entityId));
                break;
            }

            auto &objects = m_listManager.getObjects();
            RenderObjectFactory::updateResources(objects[*pIndex], *state);

            LT_LOGI("Renderer",
                    "Applied diff: Updated entity " + std::to_string(change.entityId) + " (resources changed)");
            if (!state->material.materialID.empty())
            {
                LT_LOGI("Renderer", "Material updated: " + state->material.materialID.str());
            }
            break;
        }

        case RenderDiff::EntityChange::Removed: {
            auto *state = m_entityTracker.getState(change.entityId);
            if (!state)
            {
                LT_LOGI("Renderer",
                        "Applied diff: Removed entity " + std::to_string(change.entityId) + " (already removed)");
                break;
            }

            m_listManager.removeObject(change.entityId);
            m_entityTracker.removeState(change.entityId);

            LT_LOGI("Renderer", "Applied diff: Removed entity " + std::to_string(change.entityId));
            break;
        }
        }
    }
}

void IRenderer::updateLightsFromECS()
{
    auto *worldPtr = m_ecsModule->getCurrentWorld();
    if (!worldPtr)
        return;

    auto &world = worldPtr->get();
    auto &scene = RenderLocator::Get()->scene();
    auto *ctxPtr = RenderLocator::Get();

    scene.pointLights.clear();

    // ============================================================
    // ============================================================
    {
        if (world.component<DirectionalLightComponent>().is_valid())
        {
            auto qDirLight = world.query<DirectionalLightComponent, TransformComponent>();
            bool foundLight = false;
            
            qDirLight.each(
                [&](flecs::entity e, const DirectionalLightComponent& light, const TransformComponent& transform)
                {
                    foundLight = true;
                    
                    glm::quat rotation = transform.rotation.toQuat();
                    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
                    glm::vec3 direction = rotation * forward;
                    
                    scene.sun.direction = glm::vec4(glm::normalize(direction), 0.0f);
                    scene.sun.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // Default white
                    scene.sun.intensity = light.intencity;
                    
                    glm::vec3 lightPos = transform.position.toGLMVec();
                    
                    glm::vec3 lightDir = glm::normalize(direction);
                    glm::vec3 target = lightPos + lightDir * 100.0f;
                    scene.sun.lightViewMatrix = glm::lookAt(lightPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
                    
                    const float orthoSize = 100.0f;
                    scene.sun.lightProjectionMatrix = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 500.0f);
                    
                    if (ctxPtr)
                    {
                        glm::vec3 lightPos = transform.position.toGLMVec();
                        glm::vec3 lightDir = -glm::normalize(direction);
                        float arrowLength = 10.0f;
                        glm::vec3 arrowEnd = lightPos + lightDir * arrowLength;
                        
                        DebugLine dirLine;
                        dirLine.from = lightPos;
                        dirLine.to = arrowEnd;
                        dirLine.color = glm::vec3(1.0f, 1.0f, 0.0f);
                        ctxPtr->addDebugLine(dirLine);
                        
                        float arrowHeadSize = 1.0f;
                        glm::vec3 perpendicular1 = glm::normalize(glm::cross(lightDir, glm::vec3(0.0f, 1.0f, 0.0f)));
                        if (glm::length(perpendicular1) < 0.1f)
                            perpendicular1 = glm::normalize(glm::cross(lightDir, glm::vec3(1.0f, 0.0f, 0.0f)));
                        glm::vec3 perpendicular2 = glm::normalize(glm::cross(lightDir, perpendicular1));
                        
                        DebugLine arrow1, arrow2, arrow3;
                        arrow1.from = arrowEnd;
                        arrow1.to = arrowEnd - lightDir * arrowHeadSize + perpendicular1 * arrowHeadSize * 0.3f;
                        arrow1.color = glm::vec3(1.0f, 1.0f, 0.0f);
                        ctxPtr->addDebugLine(arrow1);
                        
                        arrow2.from = arrowEnd;
                        arrow2.to = arrowEnd - lightDir * arrowHeadSize - perpendicular1 * arrowHeadSize * 0.3f;
                        arrow2.color = glm::vec3(1.0f, 1.0f, 0.0f);
                        ctxPtr->addDebugLine(arrow2);
                        
                        arrow3.from = arrowEnd;
                        arrow3.to = arrowEnd - lightDir * arrowHeadSize + perpendicular2 * arrowHeadSize * 0.3f;
                        arrow3.color = glm::vec3(1.0f, 1.0f, 0.0f);
                        ctxPtr->addDebugLine(arrow3);
                    }
                });
            
            if (!foundLight)
            {
                scene.sun.direction = glm::vec4(0.f, -1.f, 0.f, 0.f);
                scene.sun.color = glm::vec4(1.f, 1.f, 1.f, 1.f);
                scene.sun.intensity = 1.f;
                
                // Default light matrices
                scene.sun.lightViewMatrix = glm::lookAt(glm::vec3(0.0f, 50.0f, 0.0f), 
                                                        glm::vec3(0.0f, 0.0f, 0.0f), 
                                                        glm::vec3(0.0f, 0.0f, 1.0f));
                const float orthoSize = 100.0f;
                scene.sun.lightProjectionMatrix = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 500.0f);
            }
        }
        else
        {
            scene.sun.direction = glm::vec4(0.f, -1.f, 0.f, 0.f);
            scene.sun.color = glm::vec4(1.f, 1.f, 1.f, 1.f);
            scene.sun.intensity = 1.f;
            
            // Default light matrices
            scene.sun.lightViewMatrix = glm::lookAt(glm::vec3(0.0f, 50.0f, 0.0f), 
                                                    glm::vec3(0.0f, 0.0f, 0.0f), 
                                                    glm::vec3(0.0f, 0.0f, 1.0f));
            const float orthoSize = 100.0f;
            scene.sun.lightProjectionMatrix = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 500.0f);
        }
    }

    // ============================================================
    // ============================================================
    {
        if (world.component<PointLightComponent>().is_valid())
        {
            auto qPoint = world.query<PointLightComponent, TransformComponent>();
            qPoint.each(
                [&](flecs::entity e, const PointLightComponent& light, const TransformComponent& transform)
                {
                    PointLightRenderData pointLight;
                    pointLight.position = glm::vec4(transform.position.toGLMVec(), 1.0f);
                    pointLight.color = glm::vec4(light.color, 1.0f);
                    pointLight.intensity = (light.intencity > 0.0f) ? light.intencity : 1.0f;
                    pointLight.innerRadius = light.innerRadius;
                    pointLight.outerRadius = (light.outerRadius > 0.0f) ? light.outerRadius : 10.0f;
                    
                    scene.pointLights.push_back(pointLight);
                    
                    if (ctxPtr)
                    {
                        glm::vec3 lightPos = transform.position.toGLMVec();
                        float outerRadius = light.outerRadius;
                        
                        DebugSphere sphere;
                        sphere.center = lightPos;
                        sphere.radius = outerRadius;
                        sphere.color = glm::vec3(light.color.x, light.color.y, light.color.z);
                        ctxPtr->addDebugSphere(sphere);
                        
                        if (light.innerRadius > 0.0f)
                        {
                            DebugSphere innerSphere;
                            innerSphere.center = lightPos;
                            innerSphere.radius = light.innerRadius;
                            innerSphere.color = glm::vec3(light.color.x * 1.5f, light.color.y * 1.5f, light.color.z * 1.5f);
                            ctxPtr->addDebugSphere(innerSphere);
                        }
                        
                        float crossSize = 0.5f;
                        DebugLine crossX1, crossY1, crossZ1;
                        
                        crossX1.from = lightPos - glm::vec3(crossSize, 0.0f, 0.0f);
                        crossX1.to = lightPos + glm::vec3(crossSize, 0.0f, 0.0f);
                        crossX1.color = glm::vec3(1.0f, 0.0f, 0.0f);
                        ctxPtr->addDebugLine(crossX1);
                        
                        crossY1.from = lightPos - glm::vec3(0.0f, crossSize, 0.0f);
                        crossY1.to = lightPos + glm::vec3(0.0f, crossSize, 0.0f);
                        crossY1.color = glm::vec3(0.0f, 1.0f, 0.0f);
                        ctxPtr->addDebugLine(crossY1);
                        
                        crossZ1.from = lightPos - glm::vec3(0.0f, 0.0f, crossSize);
                        crossZ1.to = lightPos + glm::vec3(0.0f, 0.0f, crossSize);
                        crossZ1.color = glm::vec3(0.0f, 0.0f, 1.0f);
                        ctxPtr->addDebugLine(crossZ1);
                    }
                });
        }
    }
}

void IRenderer::rebuildRenderList()
{
    ZoneScopedN("IRenderer::rebuildRenderList");
    LT_ASSERT_MSG(m_ecsModule, "ECSModule is null");

    auto *worldPtr = m_ecsModule->getCurrentWorld();
    if (!worldPtr)
    {
        return;
    }

    auto &world = worldPtr->get();

    m_listManager.clear();
    m_entityTracker.clear();

    auto qMesh = world.query<TransformComponent, MeshComponent>();
    qMesh.each([&](flecs::entity e, const TransformComponent &transform, const MeshComponent &mesh) {
        auto &state = m_entityTracker.getOrCreateState(e.id());
        state.entityId = e.id();
        state.isValid = true;
        state.position = transform.position;
        state.rotation = transform.rotation;
        state.scale = transform.scale;
        state.mesh = mesh;
        
        const MaterialComponent* material = e.get<MaterialComponent>();
        if (material)
        {
            state.material = *material;
            if (!state.material.materialID.empty())
            {
                LT_LOGI("Renderer", "Entity " + std::to_string(e.id()) + " has material: " + state.material.materialID.str());
            }
        }
        else
        {
            state.material = MaterialComponent{};
        }

        RenderObject obj = RenderObjectFactory::createFromState(state);
        m_listManager.addObject(obj, e.id());
    });

    LT_LOGI("Renderer", "Rebuilt render list with " + std::to_string(m_listManager.size()) + " objects");
}

} // namespace RenderModule
