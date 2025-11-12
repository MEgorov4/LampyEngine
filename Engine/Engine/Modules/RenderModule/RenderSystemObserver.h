#pragma once

#include <EngineMinimal.h>
#include <flecs.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include "RenderEntityTracker.h"

namespace RenderModule
{
class RenderSystemObserver
{
private:
    flecs::world* m_world = nullptr;
    RenderEntityTracker* m_tracker = nullptr;
    
    RenderDiff m_currentDiff;
    
public:
    RenderSystemObserver() = default;
    ~RenderSystemObserver() = default;
    
    void initialize(flecs::world& world, RenderEntityTracker& tracker)
    {
        m_world = &world;
        m_tracker = &tracker;
        registerObservers();
    }
    
    RenderDiff consumeDiff()
    {
        RenderDiff diff = std::move(m_currentDiff);
        m_currentDiff.clear();
        return diff;
    }
    
    void clearDiff()
    {
        m_currentDiff.clear();
    }
    
private:
    void registerObservers()
    {
        if (!m_world)
            return;
            
        flecs::world& world = *m_world;
        
        world.observer<MeshComponent>()
            .event(flecs::OnAdd)
            .each([this](flecs::entity e, MeshComponent& mesh) {
                LT_LOGI("RenderObserver", "OnAdd MeshComponent for entity " + std::to_string(e.id()));
                onEntityBecameRenderable(e);
            });
            
        world.observer<MeshComponent>()
            .event(flecs::OnRemove)
            .each([this](flecs::entity e, MeshComponent&) {
                onEntityBecameNonRenderable(e);
            });
            
        world.observer<MeshComponent>()
            .event(flecs::OnSet)
            .each([this](flecs::entity e, MeshComponent& mesh) {
                LT_LOGI("RenderObserver", "OnSet MeshComponent for entity " + std::to_string(e.id()) + 
                        " (meshID: " + mesh.meshID.str() + ")");
                onMeshComponentChanged(e, mesh);
            });
        
        world.observer<MeshComponent>()
            .event(flecs::OnUpdate)
            .each([this](flecs::entity e, MeshComponent& mesh) {
                onMeshComponentChanged(e, mesh);
            });
        
        world.observer<PositionComponent>()
            .event(flecs::OnAdd)
            .each([this](flecs::entity e, PositionComponent&) {
                checkIfEntityBecameRenderable(e);
            });
        
        world.observer<RotationComponent>()
            .event(flecs::OnAdd)
            .each([this](flecs::entity e, RotationComponent&) {
                checkIfEntityBecameRenderable(e);
            });
        
        world.observer<ScaleComponent>()
            .event(flecs::OnAdd)
            .each([this](flecs::entity e, ScaleComponent&) {
                checkIfEntityBecameRenderable(e);
            });
        
        world.observer<MaterialComponent>()
            .event(flecs::OnAdd)
            .event(flecs::OnSet)
            .event(flecs::OnUpdate)
            .each([this](flecs::entity e, MaterialComponent& material) {
                onMaterialComponentChanged(e, material);
            });
            
    }
    
    void onEntityBecameRenderable(flecs::entity e)
    {
        bool hasPos = e.has<PositionComponent>();
        bool hasRot = e.has<RotationComponent>();
        bool hasScale = e.has<ScaleComponent>();
        bool hasMesh = e.has<MeshComponent>();
        
        if (!hasPos || !hasRot || !hasScale || !hasMesh)
        {
            LT_LOGI("RenderObserver", "Entity " + std::to_string(e.id()) + 
                    " missing components - pos:" + std::to_string(hasPos) + 
                    " rot:" + std::to_string(hasRot) + 
                    " scale:" + std::to_string(hasScale) + 
                    " mesh:" + std::to_string(hasMesh));
            return;
        }
        
        auto& state = m_tracker->getOrCreateState(e.id());
        state.entityId = e.id();
        state.isValid = true;
        
        const PositionComponent* pos = e.get<PositionComponent>();
        const RotationComponent* rot = e.get<RotationComponent>();
        const ScaleComponent* scale = e.get<ScaleComponent>();
        const MeshComponent* mesh = e.get<MeshComponent>();
        const MaterialComponent* material = e.get<MaterialComponent>();
        
        if (pos) state.position = *pos;
        if (rot) state.rotation = *rot;
        if (scale) state.scale = *scale;
        if (mesh) state.mesh = *mesh;
        if (material) state.material = *material;
        
        RenderDiff::EntityChange change;
        change.type = RenderDiff::EntityChange::Added;
        change.entityId = e.id();
        change.newState = std::make_unique<EntityRenderState>();
        change.newState->entityId = state.entityId;
        change.newState->isValid = state.isValid;
        change.newState->position = state.position;
        change.newState->rotation = state.rotation;
        change.newState->scale = state.scale;
        change.newState->mesh = state.mesh;
        change.newState->material = state.material;
        
        m_currentDiff.changes.push_back(std::move(change));
        
        LT_LOGI("RenderObserver", "Entity " + std::to_string(e.id()) + " became renderable");
    }
    
    void onEntityBecameNonRenderable(flecs::entity e)
    {
        if (!m_tracker->getState(e.id()))
            return;
        
        RenderDiff::EntityChange change;
        change.type = RenderDiff::EntityChange::Removed;
        change.entityId = e.id();
        
        m_currentDiff.changes.push_back(std::move(change));
        
        m_tracker->removeState(e.id());
        
        LT_LOGI("RenderObserver", "Entity " + std::to_string(e.id()) + " became non-renderable");
    }
    
    
    void onMaterialComponentChanged(flecs::entity e, const MaterialComponent& material)
    {
        auto* state = m_tracker->getState(e.id());
        if (!state)
        {
            return;
        }
        
        state->material = material;
        
        RenderDiff::EntityChange change;
        change.type = RenderDiff::EntityChange::Updated;
        change.entityId = e.id();
        change.newState = std::make_unique<EntityRenderState>();
        change.newState->entityId = state->entityId;
        change.newState->isValid = state->isValid;
        change.newState->position = state->position;
        change.newState->rotation = state->rotation;
        change.newState->scale = state->scale;
        change.newState->mesh = state->mesh;
        change.newState->material = state->material;
        
        m_currentDiff.changes.push_back(std::move(change));
        
        LT_LOGI("RenderObserver", "MaterialComponent changed for entity " + std::to_string(e.id()));
    }
    
    void onMeshComponentChanged(flecs::entity e, const MeshComponent& mesh)
    {
        auto* state = m_tracker->getState(e.id());
        if (!state)
        {
            if (e.has<PositionComponent>() && e.has<RotationComponent>() && 
                e.has<ScaleComponent>() && e.has<MeshComponent>())
            {
                onEntityBecameRenderable(e);
            }
            else
            {
                auto& newState = m_tracker->getOrCreateState(e.id());
                newState.entityId = e.id();
                newState.isValid = false;
                
                const PositionComponent* pos = e.get<PositionComponent>();
                const RotationComponent* rot = e.get<RotationComponent>();
                const ScaleComponent* scale = e.get<ScaleComponent>();
                
                if (pos) newState.position = *pos;
                if (rot) newState.rotation = *rot;
                if (scale) newState.scale = *scale;
                newState.mesh = mesh;
                
                LT_LOGI("RenderObserver", "MeshComponent added to entity " + std::to_string(e.id()) + 
                        " but not all transform components present yet (meshID: " + mesh.meshID.str() + ")");
            }
            return;
        }
        
        const PositionComponent* pos = e.get<PositionComponent>();
        const RotationComponent* rot = e.get<RotationComponent>();
        const ScaleComponent* scale = e.get<ScaleComponent>();
        if (pos) state->position = *pos;
        if (rot) state->rotation = *rot;
        if (scale) state->scale = *scale;
        
        bool meshChanged = state->hasMeshChanged(mesh);
        
        bool needsToBecomeRenderable = !state->isValid && 
                                       e.has<PositionComponent>() && 
                                       e.has<RotationComponent>() && 
                                       e.has<ScaleComponent>();
        
        if (meshChanged || needsToBecomeRenderable)
        {
            state->mesh = mesh;
            
            if (needsToBecomeRenderable)
            {
                state->isValid = true;
            }
            
            RenderDiff::EntityChange change;
            change.type = needsToBecomeRenderable ? RenderDiff::EntityChange::Added : RenderDiff::EntityChange::Updated;
            change.entityId = e.id();
            change.newState = std::make_unique<EntityRenderState>();
            change.newState->entityId = state->entityId;
            change.newState->isValid = state->isValid;
            change.newState->position = state->position;
            change.newState->rotation = state->rotation;
            change.newState->scale = state->scale;
            change.newState->mesh = state->mesh;
            change.newState->material = state->material;
            
            m_currentDiff.changes.push_back(std::move(change));
            
            LT_LOGI("RenderObserver", (needsToBecomeRenderable ? "Entity " : "Mesh changed for entity ") + 
                    std::to_string(e.id()) + 
                    " (meshID: " + mesh.meshID.str() + 
                    ", textureID: " + mesh.textureID.str() + ")");
        }
    }
    
    void checkIfEntityBecameRenderable(flecs::entity e)
    {
        if (!e.has<MeshComponent>())
            return;
        
        LT_LOGI("RenderObserver", "checkIfEntityBecameRenderable called for entity " + std::to_string(e.id()));
        
        auto* state = m_tracker->getState(e.id());
        if (state && !state->isValid)
        {
            if (e.has<PositionComponent>() && e.has<RotationComponent>() && e.has<ScaleComponent>())
            {
                const MeshComponent* mesh = e.get<MeshComponent>();
                if (mesh)
                {
                    onMeshComponentChanged(e, *mesh);
                }
            }
        }
        else if (!state)
        {
            if (e.has<PositionComponent>() && e.has<RotationComponent>() && e.has<ScaleComponent>())
            {
                onEntityBecameRenderable(e);
            }
        }
    }
    
};

} // namespace RenderModule

