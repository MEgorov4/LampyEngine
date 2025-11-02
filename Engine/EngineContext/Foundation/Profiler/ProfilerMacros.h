#include "ProfileScope.h"
#include "Profiler.h"
#ifdef TRACY_ENABLE
#define LT_PROFILE_FRAME_BEGIN() ::EngineCore::Foundation::Profiler::BeginFrame()
#define LT_PROFILE_FRAME_END() ::EngineCore::Foundation::Profiler::EndFrame()
#define LT_PROFILE_ZONE(name) ::EngineCore::Foundation::Profiler::BeginZone(name)
#define LT_PROFILE_MARK(msg) ::EngineCore::Foundation::Profiler::MarkText(msg)
#define LT_PROFILE_SCOPE(name) ::EngineCore::Foundation::ProfileScope __lt_scope_##__LINE__(name)

#define LT_PROFILE_ALLOC(ptr, size) ::EngineCore::Foundation::Profiler::Alloc(ptr, size)
#define LT_PROFILE_FREE(ptr) ::EngineCore::Foundation::Profiler::Free(ptr)
#define LT_PROFILE_ALLOC_N(ptr, size, tag) ::EngineCore::Foundation::Profiler::Alloc(ptr, size, tag)
#define LT_PROFILE_FREE_N(ptr, tag) ::EngineCore::Foundation::Profiler::Free(ptr, tag)
#else
#define LT_PROFILE_FRAME_BEGIN()
#define LT_PROFILE_FRAME_END()
#define LT_PROFILE_ZONE(name)
#define LT_PROFILE_MARK(msg)
#define LT_PROFILE_SCOPE(name)

#define LT_PROFILE_ALLOC(ptr, size)
#define LT_PROFILE_FREE(ptr)
#define LT_PROFILE_ALLOC_N(ptr, size, tag)
#define LT_PROFILE_FREE_N(ptr, tag)
#endif
