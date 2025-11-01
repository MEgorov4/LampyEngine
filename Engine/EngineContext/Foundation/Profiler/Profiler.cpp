#include "Profiler.h"

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

using namespace EngineCore::Foundation;

static bool frameStarted = false;

void Profiler::BeginFrame() noexcept
{
#ifdef TRACY_ENABLE
    if (!TracyIsConnected)
        return;
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
