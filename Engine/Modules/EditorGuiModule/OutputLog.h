#pragma once

#include "../ImGuiModule/GUIObject.h"
#include "Foundation/Profiler/ProfileAllocator.h"

#include <EngineMinimal.h>

/// <summary>
/// A GUI component for displaying log messages in an ImGui window.
/// This class subscribes to the Logger module and updates the UI with log messages.
/// </summary>
class GUIOutputLog : public ImGUIModule::GUIObject
{
    std::vector<std::string, ProfileAllocator<std::string>> m_messages; ///< Stores log messages to be displayed.
    int m_subscriberID;                                                 ///< ID for unsubscribing from the Logger event.

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

    /// <summary>
    /// Receives a log message from the Logger and stores it for display.
    /// </summary>
    /// <param name="message">The log message to add.</param>
    void receiveLogMessage(const std::string& message);

    /// <summary>
    /// Clears all log messages from the display.
    /// </summary>
    void clear();
};
