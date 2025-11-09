#pragma once

#include "BaseResource.h"

#include <string>

namespace ResourceModule
{
class RScript : public BaseResource
{
  public:
    explicit RScript(const std::string &path);
    ~RScript() noexcept = default;

    const std::string &getSource() const noexcept { return m_source; }

  private:
    std::string m_source;
};
} // namespace ResourceModule

