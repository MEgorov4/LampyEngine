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

    bool isValid() const noexcept
    {
        return !m_jsonData.empty();
    }

    bool isEmpty() const noexcept
    {
        return m_jsonData.empty();
    }

    void setJsonData(std::string json) noexcept
    {
        m_jsonData = std::move(json);
    }

  private:
    std::string m_jsonData;
};
} // namespace ResourceModule