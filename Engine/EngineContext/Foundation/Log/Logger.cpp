#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace EngineCore::Foundation;

LTLogger& LTLogger::Instance()
{
    static LTLogger instance;
    if (instance.m_sinks.empty())
        instance.m_sinks.push_back(std::make_shared<ConsoleSink>());
    return instance;
}

void LTLogger::addSink(std::shared_ptr<ILogSink> sink)
{
    std::scoped_lock lock(m_mutex);
    m_sinks.push_back(std::move(sink));
}

void LTLogger::clearSinks()
{
    std::scoped_lock lock(m_mutex);
    m_sinks.clear();
}

void LTLogger::log(LogVerbosity level, const std::string& category, const std::string& message)
{
    std::scoped_lock lock(m_mutex);
    for (auto& sink : m_sinks)
        sink->write(level, category, message);
}

void LTLogger::info(const std::string& cat, const std::string& msg)  { log(LogVerbosity::Info, cat, msg); }
void LTLogger::warn(const std::string& cat, const std::string& msg)  { log(LogVerbosity::Warning, cat, msg); }
void LTLogger::error(const std::string& cat, const std::string& msg) { log(LogVerbosity::Error, cat, msg); }
void LTLogger::debug(const std::string& cat, const std::string& msg) { log(LogVerbosity::Debug, cat, msg); }

void ConsoleSink::write(LogVerbosity level, const std::string& category, const std::string& message)
{
    static const char* names[] = { "Verbose", "Debug", "Info", "Warning", "Error", "Fatal" };

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tm);

    std::cout << std::format("[{}] [{}] [{}] {}\n", timeBuf, names[(int)level], category, message);
}
