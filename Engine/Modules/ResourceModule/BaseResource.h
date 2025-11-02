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
        // Пустой GUID допустим для опциональных ресурсов
        return GUID;
    }

    void setGUID(const std::string &newGUID)
    {
        // Пустой GUID допустим для опциональных ресурсов
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
