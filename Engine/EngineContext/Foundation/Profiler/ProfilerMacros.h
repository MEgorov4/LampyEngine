#include "Profiler.h"
#include "ProfileScope.h"
#ifdef TRACY_ENABLE
    #define LT_PROFILE_FRAME_BEGIN() ::EngineCore::Foundation::Profiler::BeginFrame()
    #define LT_PROFILE_FRAME_END()   ::EngineCore::Foundation::Profiler::EndFrame()
    #define LT_PROFILE_ZONE(name)    ::EngineCore::Foundation::Profiler::BeginZone(name)
    #define LT_PROFILE_MARK(msg)     ::EngineCore::Foundation::Profiler::MarkText(msg)
    #define LT_PROFILE_SCOPE(name) ::EngineCore::Foundation::ProfileScope __lt_scope_##__LINE__(name)
#else
    #define LT_PROFILE_FRAME_BEGIN()
    #define LT_PROFILE_FRAME_END()
    #define LT_PROFILE_ZONE(name)
    #define LT_PROFILE_MARK(msg)
    #define LT_PROFILE_SCOPE(name)
#endif