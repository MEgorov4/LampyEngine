#pragma once

#include <EngineMinimal.h>
#include <flecs.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include "RenderEntityTracker.h"

namespace RenderModule
{
/// Система observers для отслеживания изменений компонентов в ECS
/// Регистрирует observers в flecs и строит diff для рендер-листа
class RenderSystemObserver
{
private:
    flecs::world* m_world = nullptr;
    RenderEntityTracker* m_tracker = nullptr;
    
    // Хранилище изменений для текущего кадра
    RenderDiff m_currentDiff;
    
public:
    RenderSystemObserver() = default;
    ~RenderSystemObserver() = default;
    
    /// Инициализация observers в flecs world
    void initialize(flecs::world& world, RenderEntityTracker& tracker)
    {
        m_world = &world;
        m_tracker = &tracker;
        registerObservers();
    }
    
    /// Получить diff изменений и очистить его
    RenderDiff consumeDiff()
    {
        RenderDiff diff = std::move(m_currentDiff);
        m_currentDiff.clear();
        return diff;
    }
    
    /// Очистить текущий diff
    void clearDiff()
    {
        m_currentDiff.clear();
    }
    
private:
    /// Регистрация observers для отслеживания изменений компонентов
    void registerObservers()
    {
        if (!m_world)
            return;
            
        flecs::world& world = *m_world;
        
        // Observer для отслеживания добавления MeshComponent
        // Это означает, что сущность стала видимой для рендеринга
        world.observer<MeshComponent>()
            .event(flecs::OnAdd)
            .each([this](flecs::entity e, MeshComponent& mesh) {
                LT_LOGI("RenderObserver", "OnAdd MeshComponent for entity " + std::to_string(e.id()));
                onEntityBecameRenderable(e);
            });
            
        // Observer для отслеживания удаления MeshComponent
        // Это означает, что сущность больше не видима для рендеринга
        world.observer<MeshComponent>()
            .event(flecs::OnRemove)
            .each([this](flecs::entity e, MeshComponent&) {
                onEntityBecameNonRenderable(e);
            });
            
        // Observer для отслеживания изменений MeshComponent через set()
        // OnSet срабатывает при явном вызове entity.set<MeshComponent>()
        world.observer<MeshComponent>()
            .event(flecs::OnSet)
            .each([this](flecs::entity e, MeshComponent& mesh) {
                LT_LOGI("RenderObserver", "OnSet MeshComponent for entity " + std::to_string(e.id()) + 
                        " (meshID: " + mesh.meshID.str() + ")");
                onMeshComponentChanged(e, mesh);
            });
        
        // Observer для отслеживания изменений MeshComponent через get_mut() + modified()
        // OnUpdate срабатывает при вызове entity.modified<MeshComponent>() после get_mut()
        // Это важно для отслеживания изменений AssetID в редакторе
        world.observer<MeshComponent>()
            .event(flecs::OnUpdate)
            .each([this](flecs::entity e, MeshComponent& mesh) {
                onMeshComponentChanged(e, mesh);
            });
        
        // Observer для отслеживания добавления компонентов трансформации
        // Если у entity уже есть MeshComponent, но не было всех компонентов трансформации,
        // то теперь когда они добавились, entity должна стать renderable
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
        
        // Observer для отслеживания изменений MaterialComponent
        world.observer<MaterialComponent>()
            .event(flecs::OnAdd)
            .event(flecs::OnSet)
            .event(flecs::OnUpdate)
            .each([this](flecs::entity e, MaterialComponent& material) {
                onMaterialComponentChanged(e, material);
            });
            
        // Observer для отслеживания удаления сущности с MeshComponent
        // Когда сущность удаляется, она автоматически теряет все компоненты
        // Мы отслеживаем это через OnRemove для MeshComponent
    }
    
    /// Обработка: сущность стала рендерируемой (добавлен MeshComponent)
    void onEntityBecameRenderable(flecs::entity e)
    {
        // Проверяем наличие всех необходимых компонентов по отдельности
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
            return; // Не все необходимые компоненты
        }
        
        auto& state = m_tracker->getOrCreateState(e.id());
        state.entityId = e.id();
        state.isValid = true;
        
        // Загружаем компоненты из ECS
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
        
        // Добавляем в diff
        RenderDiff::EntityChange change;
        change.type = RenderDiff::EntityChange::Added;
        change.entityId = e.id();
        change.newState = std::make_unique<EntityRenderState>();
        // Копируем компоненты вручную (нельзя копировать из-за unique_ptr)
        change.newState->entityId = state.entityId;
        change.newState->isValid = state.isValid;
        change.newState->position = state.position;
        change.newState->rotation = state.rotation;
        change.newState->scale = state.scale;
        change.newState->mesh = state.mesh;
        change.newState->material = state.material;
        // renderObject будет создан позже
        
        m_currentDiff.changes.push_back(std::move(change));
        
        LT_LOGI("RenderObserver", "Entity " + std::to_string(e.id()) + " became renderable");
    }
    
    /// Обработка: сущность перестала быть рендерируемой (удален MeshComponent)
    void onEntityBecameNonRenderable(flecs::entity e)
    {
        if (!m_tracker->getState(e.id()))
            return; // Не была в трекере
        
        RenderDiff::EntityChange change;
        change.type = RenderDiff::EntityChange::Removed;
        change.entityId = e.id();
        
        m_currentDiff.changes.push_back(std::move(change));
        
        m_tracker->removeState(e.id());
        
        LT_LOGI("RenderObserver", "Entity " + std::to_string(e.id()) + " became non-renderable");
    }
    
    // Трансформации (Position, Rotation, Scale) обновляются каждый кадр через RenderFrameData,
    // поэтому методы onPositionChanged, onRotationChanged, onScaleChanged и addUpdateToDiff удалены
    
    /// Обработка: изменился MaterialComponent
    void onMaterialComponentChanged(flecs::entity e, const MaterialComponent& material)
    {
        auto* state = m_tracker->getState(e.id());
        if (!state)
        {
            // Сущности нет в трекере, игнорируем
            return;
        }
        
        // Обновляем material в состоянии
        state->material = material;
        
        // Добавляем в diff как Updated
        RenderDiff::EntityChange change;
        change.type = RenderDiff::EntityChange::Updated;
        change.entityId = e.id();
        change.newState = std::make_unique<EntityRenderState>();
        // Копируем только компоненты, renderObject создастся позже
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
    
    /// Обработка: изменился MeshComponent
    void onMeshComponentChanged(flecs::entity e, const MeshComponent& mesh)
    {
        auto* state = m_tracker->getState(e.id());
        if (!state)
        {
            // Сущности еще нет в трекере, добавим ее
            // Это может произойти, если MeshComponent был добавлен, но entity еще не попала в трекер
            // Например, если компоненты трансформации были добавлены после MeshComponent
            // или если проверка в onEntityBecameRenderable не прошла
            if (e.has<PositionComponent>() && e.has<RotationComponent>() && 
                e.has<ScaleComponent>() && e.has<MeshComponent>())
            {
                onEntityBecameRenderable(e);
            }
            else
            {
                // Компоненты еще не все на месте, но создадим состояние для будущего обновления
                auto& newState = m_tracker->getOrCreateState(e.id());
                newState.entityId = e.id();
                newState.isValid = false; // Пока не все компоненты
                
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
        
        // Обновляем трансформации из ECS перед проверкой изменений
        const PositionComponent* pos = e.get<PositionComponent>();
        const RotationComponent* rot = e.get<RotationComponent>();
        const ScaleComponent* scale = e.get<ScaleComponent>();
        if (pos) state->position = *pos;
        if (rot) state->rotation = *rot;
        if (scale) state->scale = *scale;
        
        // Проверяем, изменился ли mesh/текстура/шейдер
        bool meshChanged = state->hasMeshChanged(mesh);
        
        // Если entity была помечена как невалидная, но теперь все компоненты на месте, добавляем ее
        bool needsToBecomeRenderable = !state->isValid && 
                                       e.has<PositionComponent>() && 
                                       e.has<RotationComponent>() && 
                                       e.has<ScaleComponent>();
        
        if (meshChanged || needsToBecomeRenderable)
        {
            // Обновляем меш в состоянии
            state->mesh = mesh;
            
            // Если entity была невалидной, но теперь все компоненты на месте, помечаем как валидную
            if (needsToBecomeRenderable)
            {
                state->isValid = true;
            }
            
            // Добавляем в diff с полным состоянием
            RenderDiff::EntityChange change;
            change.type = needsToBecomeRenderable ? RenderDiff::EntityChange::Added : RenderDiff::EntityChange::Updated;
            change.entityId = e.id();
            change.newState = std::make_unique<EntityRenderState>();
            // Копируем все компоненты вручную
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
    
    /// Проверка: стала ли сущность renderable после добавления компонентов трансформации
    /// Вызывается когда добавляется Position, Rotation или Scale компонент
    void checkIfEntityBecameRenderable(flecs::entity e)
    {
        // Проверяем, есть ли у entity MeshComponent
        if (!e.has<MeshComponent>())
            return;
        
        LT_LOGI("RenderObserver", "checkIfEntityBecameRenderable called for entity " + std::to_string(e.id()));
        
        // Проверяем, есть ли уже state для этой entity
        auto* state = m_tracker->getState(e.id());
        if (state && !state->isValid)
        {
            // State существует, но была помечена как невалидная
            // Проверяем, есть ли теперь все компоненты
            if (e.has<PositionComponent>() && e.has<RotationComponent>() && e.has<ScaleComponent>())
            {
                // Теперь все компоненты на месте - делаем entity renderable
                const MeshComponent* mesh = e.get<MeshComponent>();
                if (mesh)
                {
                    onMeshComponentChanged(e, *mesh);
                }
            }
        }
        else if (!state)
        {
            // State еще не создан - попробуем добавить entity если все компоненты уже есть
            if (e.has<PositionComponent>() && e.has<RotationComponent>() && e.has<ScaleComponent>())
            {
                onEntityBecameRenderable(e);
            }
        }
    }
    
    /// Обработка: сущность удалена
    /// Это вызывается автоматически через OnRemove для MeshComponent
    /// в onEntityBecameNonRenderable, поэтому этот метод не нужен
};

} // namespace RenderModule

