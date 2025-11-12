#pragma once
#include "Logger.h"
#include "LogVerbosity.h"

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
        ::EngineCore::Foundation::GetLogger().log(::EngineCore::Foundation::LogVerbosity::Info, (category), (message));\
    } while (0)

#define LT_LOGW(category, message)                                          \
    do {                                                                          \
        ::EngineCore::Foundation::GetLogger().log(::EngineCore::Foundation::LogVerbosity::Warning, (category), (message));\
    } while (0)

#define LT_LOGE(category, message)                                          \
    do {                                                                          \
        ::EngineCore::Foundation::GetLogger().log(::EngineCore::Foundation::LogVerbosity::Error, (category), (message));\
    } while (0)

