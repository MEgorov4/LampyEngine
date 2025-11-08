#pragma once
#include <EngineMinimal.h>
#include "BaseResource.h"

namespace ResourceModule
{
class RWorld final : public BaseResource
{
  public:
    explicit RWorld(const std::string& path);
    ~RWorld() noexcept = default;

    const std::string& getJsonData() const noexcept
    {
        return m_jsonData;
    }

  private:
    std::string m_jsonData;
};
} // namespace ResourceModule