#pragma once

#include <EngineMinimal.h>
#include "RenderEntityTracker.h"
#include "RenderContext.h"
#include "Abstract/RenderObject.h"
#include "Foundation/Memory/ResourceAllocator.h"
#include <unordered_map>

using EngineCore::Foundation::ResourceAllocator;

namespace RenderModule
{
class RenderListManager
{
private:
    RenderEntityTracker& m_tracker;
    RenderContext* m_context;
    
    std::unordered_map<uint64_t, size_t> m_entityToObjectIndex;
    
    using RenderObjectList = std::vector<RenderObject, ResourceAllocator<RenderObject>>;
    
public:
    explicit RenderListManager(RenderEntityTracker& tracker);
    
    void setContext(RenderContext* context);
    
    RenderObjectList& getObjects();
    const RenderObjectList& getObjects() const;
    
    size_t addObject(const RenderObject& obj, uint64_t entityId);
    
    bool updateObject(size_t index, const RenderObject& obj);
    
    bool removeObject(uint64_t entityId);
    
    size_t* getObjectIndex(uint64_t entityId);
    
    void clear();
    
    bool isValidIndex(size_t index) const;
    
    size_t size() const;
};

} // namespace RenderModule

