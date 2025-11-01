#pragma once

#include "Profiler.h"


namespace EngineCore::Foundation
{
class ProfileScope
{
  public:
    explicit ProfileScope(const char *name) noexcept;

    ~ProfileScope() noexcept = default; // Tracy сам закроет зону
};
} // namespace EngineCore::Foundation
