#pragma once
#include <string>
#include <format>
#include <iostream>
#include <mutex>

#include "../EventModule/Event.h"

/// <summary>
/// Defines different log verbosity levels.
/// </summary>
enum class LogVerbosity
{
    Verbose,  ///< Detailed debug information.
    Debug,    ///< General debugging messages.
    Info,     ///< General information messages.
    Warning,  ///< Warnings that may indicate potential issues.
    Error,    ///< Errors that indicate failures but do not stop execution.
    Fatal     ///< Critical errors that may terminate execution.
};

/// <summary>
/// Singleton class for logging messages with different verbosity levels.
/// Supports event-based logging via the OnMessagePushed event.
/// </summary>
class Logger
{
public:
    /// <summary>
    /// Event that triggers when a new log message is pushed.
    /// Subscribers receive the formatted log message.
    /// </summary>
    Event<std::string> OnMessagePushed;

    /// <summary>
    /// Retrieves the singleton instance of the Logger.
    /// </summary>
    /// <returns>Reference to the Logger instance.</returns>
    static Logger& Get()
    {
        static Logger instance;
        return instance;
    }

    /// <summary>
    /// Logs a message with a specified verbosity level.
    /// </summary>
    /// <param name="verbosity">The verbosity level of the log.</param>
    /// <param name="message">The message to be logged.</param>
    void Log(LogVerbosity verbosity, const std::string& message);

private:
    /// <summary>
    /// Private constructor to enforce the singleton pattern.
    /// </summary>
    Logger() = default;

    /// <summary>
    /// Private destructor.
    /// </summary>
    ~Logger() = default;

    /// <summary>
    /// Deleted copy constructor to prevent copying.
    /// </summary>
    Logger(const Logger&) = delete;

    /// <summary>
    /// Deleted assignment operator to prevent copying.
    /// </summary>
    Logger& operator=(const Logger&) = delete;

    /// <summary>
    /// Converts a LogVerbosity value to a string representation.
    /// </summary>
    /// <param name="verbosity">The verbosity level.</param>
    /// <returns>String representation of the verbosity level.</returns>
    std::string ToString(LogVerbosity verbosity) const;

    std::mutex m_mutex; ///< Mutex to ensure thread-safe logging.
};

/// <summary>
/// Logs a verbose message.
/// </summary>
#define LOG_VERBOSE(Message)   Logger::Get().Log(LogVerbosity::Verbose,  (Message))

/// <summary>
/// Logs a debug message.
/// </summary>
#define LOG_DEBUG(Message)     Logger::Get().Log(LogVerbosity::Debug,    (Message))

/// <summary>
/// Logs an informational message.
/// </summary>
#define LOG_INFO(Message)      Logger::Get().Log(LogVerbosity::Info,     (Message))

/// <summary>
/// Logs a warning message.
/// </summary>
#define LOG_WARNING(Message)   Logger::Get().Log(LogVerbosity::Warning,  (Message))

/// <summary>
/// Logs an error message.
/// </summary>
#define LOG_ERROR(Message)     Logger::Get().Log(LogVerbosity::Error,    (Message))

/// <summary>
/// Logs a fatal error message.
/// </summary>
#define LOG_FATAL(Message)     Logger::Get().Log(LogVerbosity::Fatal,    (Message))
