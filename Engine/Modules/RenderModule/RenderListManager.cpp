#include "RenderListManager.h"

namespace RenderModule
{

RenderListManager::RenderListManager(RenderEntityTracker& tracker)
    : m_tracker(tracker)
    , m_context(nullptr)
{
}

void RenderListManager::setContext(RenderContext* context)
{
    m_context = context;
}

RenderListManager::RenderObjectList& RenderListManager::getObjects()
{
    if (!m_context)
        throw std::runtime_error("RenderContext is not set");
    return m_context->scene().objects;
}

const RenderListManager::RenderObjectList& RenderListManager::getObjects() const
{
    if (!m_context)
        throw std::runtime_error("RenderContext is not set");
    return m_context->scene().objects;
}

size_t RenderListManager::addObject(const RenderObject& obj, uint64_t entityId)
{
    auto& objects = getObjects();
    size_t index = objects.size();
    objects.push_back(obj);
    m_entityToObjectIndex[entityId] = index;
    return index;
}

bool RenderListManager::updateObject(size_t index, const RenderObject& obj)
{
    auto& objects = getObjects();
    if (index >= objects.size())
        return false;
    objects[index] = obj;
    return true;
}

bool RenderListManager::removeObject(uint64_t entityId)
{
    auto it = m_entityToObjectIndex.find(entityId);
    if (it == m_entityToObjectIndex.end())
        return false;
    
    size_t objIndex = it->second;
    auto& objects = getObjects();
    
    if (objIndex >= objects.size())
    {
        m_entityToObjectIndex.erase(it);
        return false;
    }
    
    // Удаляем объект из списка
    objects.erase(objects.begin() + objIndex);
    
    // Обновляем маппинг для всех объектов после удаленного
    // Все индексы после удаленного объекта сдвигаются на -1
    for (auto& [id, index] : m_entityToObjectIndex)
    {
        if (index > objIndex)
        {
            index--;
        }
    }
    
    // Удаляем из маппинга
    m_entityToObjectIndex.erase(it);
    
    return true;
}

size_t* RenderListManager::getObjectIndex(uint64_t entityId)
{
    auto it = m_entityToObjectIndex.find(entityId);
    if (it == m_entityToObjectIndex.end())
        return nullptr;
    return &it->second;
}

void RenderListManager::clear()
{
    getObjects().clear();
    m_entityToObjectIndex.clear();
}

bool RenderListManager::isValidIndex(size_t index) const
{
    return index < getObjects().size();
}

size_t RenderListManager::size() const
{
    return getObjects().size();
}

} // namespace RenderModule

