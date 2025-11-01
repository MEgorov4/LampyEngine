#include "ProfileScope.h"
using namespace EngineCore::Foundation;

#include <tracy/Tracy.hpp>

ProfileScope::ProfileScope(const char *name) noexcept
{
#ifdef TRACY_ENABLE
    ZoneTransientN(___tracy_scoped_zone, name, true);
#endif
}
