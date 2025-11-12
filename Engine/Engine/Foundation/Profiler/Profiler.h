#pragma once
#include <cstddef>

namespace EngineCore::Foundation
{
class Profiler
{
  public:
    static void BeginFrame() noexcept;
    static void EndFrame() noexcept;
    static void MarkText(const char *text) noexcept;
    static void BeginZone(const char *name) noexcept;

    static void Alloc(const void *ptr, std::size_t size, const char *name = nullptr) noexcept;
    static void Free(const void *ptr, const char *name = nullptr) noexcept;
};
} // namespace EngineCore::Foundation
