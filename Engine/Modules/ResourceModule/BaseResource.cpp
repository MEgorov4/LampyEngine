#include "BaseResource.h"


std::string BaseResource::getGUID() const
{
	return GUID;
}

void BaseResource::setGUID(std::string& newGUID)
{
	GUID = newGUID;
}
