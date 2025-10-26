#pragma once
#include "Logger.h"

namespace EngineCore::Foundation
{
    inline LTLogger& GetLogger() noexcept { return LTLogger::Instance(); }
} // namespace EngineCore::Foundation

#define LT_LOG(level, category, message)                                          \
    do {                                                                          \
        ::EngineCore::Foundation::GetLogger().log((level), (category), (message));\
    } while (0)
