#include "Assert.h"
#include "../Log/Logger.h"

#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif

namespace EngineCore::Foundation
{
void ReportAssert(std::string_view expr, std::string_view message,
                  const std::source_location loc) noexcept
{
    std::string formatted = std::format(
        "[ASSERT FAILED]\n"
        "Expr: {}\n"
        "File: {}\n"
        "Function: {}\n"
        "Line: {}\n"
        "Message: {}\n",
        expr,
        loc.file_name(),
        loc.function_name(),
        loc.line(),
        message.empty() ? "(none)" : message.data());

    LTLogger::Instance().log(LogVerbosity::Error, formatted, "Assert");

#if defined(_WIN32)
    OutputDebugStringA(formatted.c_str());
#endif
}
} // namespace EngineCore::Foundation
