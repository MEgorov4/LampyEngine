#include "Profiler.h"
#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

using namespace EngineCore::Foundation;

void Profiler::BeginFrame() noexcept
{
#ifdef TRACY_ENABLE
    FrameMark;
#endif
}

void Profiler::EndFrame() noexcept
{
}

void Profiler::MarkText(const char *text) noexcept
{
#ifdef TRACY_ENABLE
    TracyMessage(text, strlen(text));
#endif
}

void Profiler::BeginZone(const char *name) noexcept
{
#ifdef TRACY_ENABLE
    ZoneTransientN(___tracy_scoped_zone, name, true);
#endif
}

void Profiler::Alloc(const void *ptr, std::size_t size, const char *name) noexcept
{
#ifdef TRACY_ENABLE
    if (name)
        TracyAllocN(ptr, size, name);
    else
        TracyAlloc(ptr, size);
#endif
}

void Profiler::Free(const void *ptr, const char *name) noexcept
{
#ifdef TRACY_ENABLE
    if (name)
        TracyFreeN(ptr, name);
    else
        TracyFree(ptr);
#endif
}
