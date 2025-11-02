#include "Profiler.h"
#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
// TracyOpenGL.hpp removed - Profiler.cpp doesn't use GPU profiling functions
// If GPU profiling is needed here, add: #include <GL/glew.h> before TracyOpenGL.hpp
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
    // Deprecated: Use LT_PROFILE_ZONE macro directly instead
    // This function is kept for backwards compatibility but doesn't work correctly
    // because Tracy zones must have scope - use LT_PROFILE_SCOPE or LT_PROFILE_ZONE instead
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
