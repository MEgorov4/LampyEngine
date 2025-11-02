#pragma once

#include "../../EngineContext/Foundation/Assert/Assert.h"
#include <string>
#include <xstring>

namespace ResourceModule
{
class BaseResource
{
  public:
    BaseResource(const std::string &path)
    {
        LT_ASSERT_MSG(!path.empty(), "Resource path cannot be empty");
        GUID = path;
    }

    std::string getGUID() const
    {
        LT_ASSERT_MSG(!GUID.empty(), "Resource GUID is empty");
        return GUID;
    }

    void setGUID(const std::string &newGUID)
    {
        LT_ASSERT_MSG(!newGUID.empty(), "Cannot set empty GUID");
        GUID = newGUID;
    }

  protected:
    std::string GUID;
};
} // namespace ResourceModule
