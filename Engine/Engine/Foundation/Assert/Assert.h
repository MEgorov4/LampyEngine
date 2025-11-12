#pragma once

#include <format>
#include <source_location>
#include <string_view>

namespace EngineCore::Foundation
{
    void ReportAssert(
        std::string_view expr,
        std::string_view message = "",
        const std::source_location loc = std::source_location::current()) noexcept;
}

#if defined(LT_DEBUG)

    #if defined(_WIN32)
        #ifndef NOMINMAX
        #define NOMINMAX
        #endif
        #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
        #endif
    #endif

    #define LT_ASSERT(expr) \
        do { \
            if (!(expr)) { \
                ::EngineCore::Foundation::ReportAssert(#expr, "", std::source_location::current()); \
            } \
        } while (false)

    #define LT_ASSERT_MSG(expr, msg) \
        do { \
            if (!(expr)) { \
                ::EngineCore::Foundation::ReportAssert(#expr, msg, std::source_location::current()); \
            } \
        } while (false)

#else
    #define LT_ASSERT(expr)        ((void)0)
    #define LT_ASSERT_MSG(expr, m) ((void)0)
#endif
