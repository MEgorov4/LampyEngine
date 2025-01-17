#pragma once
#include <string>
#include <iostream>
#include <mutex>

enum class LogVerbosity
{
    Verbose,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class Logger
{
public:
    static Logger& Get()
    {
        static Logger instance;
        return instance;
    }

    void Log(LogVerbosity verbosity, const std::string& message)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::cout << "[" << ToString(verbosity) << "] " << message << std::endl;
    }

private:
    Logger() = default;
    ~Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string ToString(LogVerbosity verbosity) const;

    std::mutex m_mutex; 
};

#define LOG_VERBOSE(Message)   Logger::Get().Log(LogVerbosity::Verbose,  (Message))
#define LOG_DEBUG(Message)     Logger::Get().Log(LogVerbosity::Debug,    (Message))
#define LOG_INFO(Message)      Logger::Get().Log(LogVerbosity::Info,     (Message))
#define LOG_WARNING(Message)   Logger::Get().Log(LogVerbosity::Warning,  (Message))
#define LOG_ERROR(Message)     Logger::Get().Log(LogVerbosity::Error,    (Message))
#define LOG_FATAL(Message)     Logger::Get().Log(LogVerbosity::Fatal,    (Message))


