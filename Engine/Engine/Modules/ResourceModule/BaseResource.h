#pragma once

#include <EngineMinimal.h>
#include <string>
#include <xstring>

namespace ResourceModule
{
class BaseResource
{
  public:
    virtual ~BaseResource() = default;

    BaseResource(const std::string &path)
    {
        GUID = path;
    }

    std::string getGUID() const
    {
        return GUID;
    }

    void setGUID(const std::string &newGUID)
    {
        GUID = newGUID;
    }
    
    bool isEmpty() const noexcept
    {
        return GUID.empty();
    }

  protected:
    std::string GUID;
};
} // namespace ResourceModule
