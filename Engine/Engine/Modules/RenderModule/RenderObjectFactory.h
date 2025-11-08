#pragma once

#include <EngineMinimal.h>
#include "RenderEntityTracker.h"
#include "Abstract/RenderObject.h"
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <glm/glm.hpp>

namespace RenderModule
{
/// Фабрика для создания и обновления рендер-объектов
class RenderObjectFactory
{
public:
    /// Создать рендер-объект из состояния сущности
    static RenderObject createFromState(EntityRenderState& state);
    
    /// Обновить трансформацию рендер-объекта из состояния
    static void updateTransform(RenderObject& obj, const EntityRenderState& state);
    
    /// Обновить ресурсы рендер-объекта из состояния (mesh, shader, texture)
    static void updateResources(RenderObject& obj, const EntityRenderState& state);
    
private:
    /// Вычислить матрицу модели из компонентов трансформации
    static glm::mat4 computeModelMatrix(const PositionComponent& pos, 
                                        const RotationComponent& rot, 
                                        const ScaleComponent& scale);
};

} // namespace RenderModule

