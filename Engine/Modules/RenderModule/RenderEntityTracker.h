#pragma once

#include "Abstract/RenderObject.h"
#include <EngineMinimal.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

namespace RenderModule
{
/// Состояние сущности в рендер-листе
struct EntityRenderState
{
    uint64_t entityId = 0;
    bool isValid = false;

    // Компоненты, необходимые для рендеринга
    PositionComponent position{};
    RotationComponent rotation{};
    ScaleComponent scale{};
    MeshComponent mesh{};
    MaterialComponent material{}; // Material для PBR

    // Рендер-объект, созданный из компонентов
    std::unique_ptr<RenderObject> renderObject;

    // Проверка, изменились ли компоненты трансформации
    bool hasTransformChanged(const PositionComponent &pos, const RotationComponent &rot,
                             const ScaleComponent &scl) const
    {
        return position.x != pos.x || position.y != pos.y || position.z != pos.z || rotation.x != rot.x ||
               rotation.y != rot.y || rotation.z != rot.z || rotation.qx != rot.qx || rotation.qy != rot.qy ||
               rotation.qz != rot.qz || rotation.qw != rot.qw || scale.x != scl.x || scale.y != scl.y ||
               scale.z != scl.z;
    }

    // Проверка, изменился ли mesh
    bool hasMeshChanged(const MeshComponent &m) const
    {
        return mesh.meshID != m.meshID || mesh.vertShaderID != m.vertShaderID || mesh.fragShaderID != m.fragShaderID ||
               mesh.textureID != m.textureID;
    }

    // Обновить матрицу модели
    void updateModelMatrix()
    {
        if (!renderObject)
            return;

        glm::mat4 model(1.f);
        glm::vec3 posVec = position.toGLMVec();
        model = glm::translate(model, posVec);
        model *= glm::mat4_cast(rotation.toQuat());
        glm::vec3 scaleVec = scale.toGLMVec();
        model = glm::scale(model, scaleVec);
        renderObject->modelMatrix.model = model;
    }
};

/// Дифф изменения для рендер-листа
struct RenderDiff
{
    struct EntityChange
    {
        enum Type
        {
            Added,
            Updated,
            Removed
        };
        Type type;
        uint64_t entityId;
        std::unique_ptr<EntityRenderState> newState;
    };

    std::vector<EntityChange> changes;

    void clear()
    {
        changes.clear();
    }
    bool empty() const
    {
        return changes.empty();
    }
};

/// Трекер состояния сущностей для рендеринга
/// Отслеживает изменения и строит diff для инкрементальных обновлений
class RenderEntityTracker
{
  private:
    // Карта состояний сущностей по их ID
    std::unordered_map<uint64_t, EntityRenderState> m_entityStates;

  public:
    RenderEntityTracker() = default;
    ~RenderEntityTracker() = default;

    /// Получить состояние сущности (создать если не существует)
    EntityRenderState &getOrCreateState(uint64_t entityId)
    {
        auto it = m_entityStates.find(entityId);
        if (it == m_entityStates.end())
        {
            it = m_entityStates.emplace(entityId, EntityRenderState{}).first;
            it->second.entityId = entityId;
            it->second.isValid = false;
        }
        return it->second;
    }

    /// Получить состояние сущности (nullptr если не существует)
    EntityRenderState *getState(uint64_t entityId)
    {
        auto it = m_entityStates.find(entityId);
        if (it == m_entityStates.end())
            return nullptr;
        return &it->second;
    }

    /// Удалить состояние сущности
    void removeState(uint64_t entityId)
    {
        m_entityStates.erase(entityId);
    }

    /// Очистить все состояния
    void clear()
    {
        m_entityStates.clear();
    }

    /// Получить все состояния
    const std::unordered_map<uint64_t, EntityRenderState> &getStates() const
    {
        return m_entityStates;
    }
};

} // namespace RenderModule
