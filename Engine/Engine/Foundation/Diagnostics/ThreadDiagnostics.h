#pragma once

#include <EngineMinimal.h>

namespace EngineCore::Foundation::Diagnostics
{
/// Logs currently active threads in the process (platform-dependent).
void LogActiveThreads(const char* reason = nullptr) noexcept;
}


