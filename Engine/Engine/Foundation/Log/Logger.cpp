#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

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
    static const char* names[] = {"Verbose", "Debug", "Info", "Warning", "Error", "Fatal"};

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tm);

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD color      = 7; // default grey

    switch (level)
    {
    case LogVerbosity::Verbose:
        color = 8;
        break; // Dark Gray
    case LogVerbosity::Debug:
        color = 7;
        break; // Light Gray
    case LogVerbosity::Info:
        color = 10;
        break; // Green
    case LogVerbosity::Warning:
        color = 14;
        break; // Yellow
    case LogVerbosity::Error:
        color = 12;
        break; // Red
    case LogVerbosity::Fatal:
        color = 4 | 8;
        break; // Intense Red
    }

    SetConsoleTextAttribute(hConsole, color);
    std::cout << std::format("[{}] [{}] [{}] {}\n", timeBuf, names[(int) level], category, message);
    SetConsoleTextAttribute(hConsole, 7); // reset
#else
    const char* colorCode = "\033[0m"; // reset
    switch (level)
    {
    case LogVerbosity::Verbose:
        colorCode = "\033[90m";
        break; // Dark gray
    case LogVerbosity::Debug:
        colorCode = "\033[37m";
        break; // White
    case LogVerbosity::Info:
        colorCode = "\033[32m";
        break; // Green
    case LogVerbosity::Warning:
        colorCode = "\033[33m";
        break; // Yellow
    case LogVerbosity::Error:
        colorCode = "\033[31m";
        break; // Red
    case LogVerbosity::Fatal:
        colorCode = "\033[1;31m";
        break; // Bold red
    }

    std::cout << std::format("{}[{}] [{}] [{}] {}\033[0m\n", colorCode, timeBuf, names[(int) level], category, message);
#endif
}
