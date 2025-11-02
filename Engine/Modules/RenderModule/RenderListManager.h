#pragma once

#include <EngineMinimal.h>
#include "RenderEntityTracker.h"
#include "RenderContext.h"
#include "Abstract/RenderObject.h"
#include "Foundation/Profiler/ProfileAllocator.h"
#include <unordered_map>

namespace RenderModule
{
/// Менеджер рендер-листа объектов
/// Отвечает за синхронизацию списка объектов с ECS и управление маппингом индексов
class RenderListManager
{
private:
    RenderEntityTracker& m_tracker;
    RenderContext* m_context;
    
    // Маппинг entityId -> индекс объекта в списке объектов (для быстрого поиска)
    std::unordered_map<uint64_t, size_t> m_entityToObjectIndex;
    
    // Тип для списка объектов с ProfileAllocator (как в RenderScene)
    using RenderObjectList = std::vector<RenderObject, ProfileAllocator<RenderObject>>;
    
public:
    explicit RenderListManager(RenderEntityTracker& tracker);
    
    /// Установить контекст рендеринга
    void setContext(RenderContext* context);
    
    /// Получить список объектов из сцены
    RenderObjectList& getObjects();
    const RenderObjectList& getObjects() const;
    
    /// Добавить объект в список и вернуть индекс
    size_t addObject(const RenderObject& obj, uint64_t entityId);
    
    /// Обновить объект по индексу
    bool updateObject(size_t index, const RenderObject& obj);
    
    /// Удалить объект по entityId
    bool removeObject(uint64_t entityId);
    
    /// Получить индекс объекта по entityId
    size_t* getObjectIndex(uint64_t entityId);
    
    /// Очистить весь список
    void clear();
    
    /// Проверить валидность индекса
    bool isValidIndex(size_t index) const;
    
    /// Получить количество объектов
    size_t size() const;
};

} // namespace RenderModule

