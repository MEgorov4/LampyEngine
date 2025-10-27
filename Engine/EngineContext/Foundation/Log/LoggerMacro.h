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


#define LT_LOGI(category, message)                                          \
    do {                                                                          \
        ::EngineCore::Foundation::GetLogger().log((LogVerbosity::Info), (category), (message));\
    } while (0)

#define LT_LOGE(category, message)                                          \
    do {                                                                          \
        ::EngineCore::Foundation::GetLogger().log((LogVerbosity::Error), (category), (message));\
    } while (0)

