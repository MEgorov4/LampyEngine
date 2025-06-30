/*#include "GarbageAllocator.h"

#include <memory>
#include "DoubleStackAllocator.h"
#include "../ResourceModule/Material.h"

#include "../ResourceModule/ResourceManager.h"

template<typename T>
inline GarbageAllocator<T>::GarbageAllocator(DoubleStackAllocator* doubleStack) : m_doubleStack(doubleStack)
{

}

template<typename T>
T* GarbageAllocator<T>::innerAllocate(size_t n)
{
    if (ResourceManager::getInstance().getDoubleStackAllocator())
    {
        void* ptr = ResourceManager::getInstance().getDoubleStackAllocator()->allocateEnd(n * sizeof(T));

        return static_cast<T*>(ptr);
    }
    
    return nullptr;
}

template class GarbageAllocator<char>;
template class GarbageAllocator<uint8_t>;
template class GarbageAllocator<uint32_t>;
template class GarbageAllocator<RMaterial*>;
template class GarbageAllocator<MeshVertex>;
template class GarbageAllocator<std::_Container_proxy>;*/