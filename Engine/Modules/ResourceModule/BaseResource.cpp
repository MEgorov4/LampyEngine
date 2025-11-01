#include "BaseResource.h"

namespace ResourceModule
{
std::string BaseResource::getGUID() const
{
    return GUID;
}

void BaseResource::setGUID(const std::string& newGUID)
{
    GUID = newGUID;
}
} // namespace ResourceModule
