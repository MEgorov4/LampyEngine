#include "ThreadDiagnostics.h"

#include <Foundation/Log/LoggerMacro.h>
#include <Foundation/Log/LogVerbosity.h>

#include <string>
#include <vector>
#include <format>

#if defined(_WIN32)
#include <windows.h>
#include <tlhelp32.h>
#endif

namespace EngineCore::Foundation::Diagnostics
{
void LogActiveThreads(const char* reason) noexcept
{
#if defined(_WIN32)
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        LT_LOGW("ThreadDiag", "Unable to enumerate threads (CreateToolhelp32Snapshot failed)");
        return;
    }

    THREADENTRY32 entry{};
    entry.dwSize = sizeof(entry);

    if (!Thread32First(snapshot, &entry))
    {
        CloseHandle(snapshot);
        LT_LOGW("ThreadDiag", "Unable to enumerate threads (Thread32First failed)");
        return;
    }

    const DWORD currentPid = GetCurrentProcessId();
    std::vector<DWORD> threadIds;

    do
    {
        if (entry.th32OwnerProcessID == currentPid)
        {
            threadIds.push_back(entry.th32ThreadID);
        }
    } while (Thread32Next(snapshot, &entry));

    CloseHandle(snapshot);

    const char* label = reason ? reason : "Thread snapshot";
    LT_LOGI("ThreadDiag", std::format("{}: {} thread(s) alive", label, threadIds.size()));

    if (threadIds.size() > 1)
    {
        std::string list;
        list.reserve(threadIds.size() * 6);
        for (std::size_t i = 0; i < threadIds.size(); ++i)
        {
            if (!list.empty())
                list += ", ";
            list += std::to_string(threadIds[i]);
        }
        LT_LOGW("ThreadDiag", std::format("Remaining thread IDs: {}", list));
    }
#else
    (void)reason;
#endif
}
} // namespace EngineCore::Foundation::Diagnostics


