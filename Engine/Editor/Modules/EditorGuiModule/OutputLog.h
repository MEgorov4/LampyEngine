#pragma once

#include <Modules/ImGuiModule/GUIObject.h> 
#include "Foundation/Log/LogVerbosity.h"
#include "Foundation/Log/Logger.h"
#include "Foundation/Profiler/ProfileAllocator.h"
#include <imgui.h>

#include <EngineMinimal.h>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace EngineCore::Foundation
{
class LTLogger;
}

/// <summary>
/// Structured log message with verbosity, category, and timestamp
/// </summary>
struct LogMessage
{
    EngineCore::Foundation::LogVerbosity verbosity;
    std::string category;
    std::string message;
    std::chrono::system_clock::time_point timestamp;

    LogMessage(EngineCore::Foundation::LogVerbosity v, const std::string &cat, const std::string &msg)
        : verbosity(v), category(cat), message(msg), timestamp(std::chrono::system_clock::now())
    {
    }
};

/// <summary>
/// Log sink implementation for GUI output
/// </summary>
class GUILogSink final : public EngineCore::Foundation::ILogSink
{
  public:
    GUILogSink(class GUIOutputLog *outputLog) : m_outputLog(outputLog)
    {
    }
    void write(EngineCore::Foundation::LogVerbosity level, const std::string &category,
               const std::string &message) override;

  private:
    class GUIOutputLog *m_outputLog;
};

/// <summary>
/// A GUI component for displaying log messages in an ImGui window.
/// This class subscribes to the Logger module and updates the UI with log messages.
/// </summary>
class GUIOutputLog : public ImGUIModule::GUIObject
{
    friend class GUILogSink;

    // Message storage
    std::vector<LogMessage, ProfileAllocator<LogMessage>> m_messages; ///< Stores log messages to be displayed.
    std::mutex m_messagesMutex;                                       ///< Mutex for thread-safe message access

    // Filtering
    char m_categoryFilter[256] = ""; ///< Category search filter
    bool m_verbosityFilters[6] = {
        true, true, true, true, true, true}; ///< Verbosity level filters (Verbose, Debug, Info, Warning, Error, Fatal)

    // UI state
    bool m_autoScroll = true; ///< Automatically scroll to bottom
    int m_maxMessages = 1000; ///< Maximum number of messages to store

    // Sink registration
    std::shared_ptr<GUILogSink> m_logSink; ///< Log sink registered with the logger

  public:
    /// <summary>
    /// Constructs a GUI log output panel and subscribes to the Logger event.
    /// </summary>
    GUIOutputLog();

    /// <summary>
    /// Destroys the log output panel and unsubscribes from the Logger event.
    /// </summary>
    ~GUIOutputLog() override;

    /// <summary>
    /// Renders the log messages inside an ImGui window.
    /// </summary>
    void render(float deltaTime) override;

  private:
    /// <summary>
    /// Clears all log messages.
    /// </summary>
    void clear();

    /// <summary>
    /// Receives a log message and adds it to the message list.
    /// </summary>
    void receiveLogMessage(EngineCore::Foundation::LogVerbosity verbosity, const std::string &category,
                           const std::string &message);

    /// <summary>
    /// Checks if a message should be displayed based on filters
    /// </summary>
    bool shouldDisplayMessage(const LogMessage &msg) const;

    /// <summary>
    /// Gets ImGui color for a verbosity level
    /// </summary>
    ImVec4 getColorForVerbosity(EngineCore::Foundation::LogVerbosity verbosity) const;
};
