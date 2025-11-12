#include "OutputLog.h"
#include "Foundation/Log/Logger.h"
#include <algorithm>
#include <ctime>
#include <imgui.h>
#include <imgui_internal.h>
#include <iomanip>
#include <sstream>

using namespace EngineCore::Foundation;

// GUILogSink implementation
void GUILogSink::write(LogVerbosity level, const std::string &category, const std::string &message)
{
    if (m_outputLog)
    {
        m_outputLog->receiveLogMessage(level, category, message);
    }
}

// GUIOutputLog implementation
GUIOutputLog::GUIOutputLog() : GUIObject()
{
    // Create and register the log sink
    m_logSink = std::make_shared<GUILogSink>(this);
    LTLogger::Instance().addSink(m_logSink);
}

GUIOutputLog::~GUIOutputLog()
{
    // Note: We can't easily remove a sink from LTLogger as there's no removeSink method
    // The sink will be destroyed automatically when shared_ptr count reaches 0
    // In a production system, you'd want to add removeSink() method to LTLogger
}

void GUIOutputLog::render(float deltaTime)
{
    ZoneScopedN("GUIObject::OutputLog");
    if (!isVisible())
        return;

    bool windowOpen = true;
    if (ImGui::Begin("Console Log", &windowOpen, ImGuiWindowFlags_None))
    {
        // Toolbar with filters and buttons
        // Clear button
        if (ImGui::Button("Clear"))
        {
            clear();
        }

        ImGui::SameLine();

        // Auto-scroll checkbox
        ImGui::Checkbox("Auto-scroll", &m_autoScroll);

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        // Category filter
        ImGui::Text("Category:");
        ImGui::SameLine();
        ImGui::PushItemWidth(150);
        ImGui::InputText("##CategoryFilter", m_categoryFilter, sizeof(m_categoryFilter));
        ImGui::PopItemWidth();

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        // Verbosity filters
        ImGui::Text("Levels:");
        ImGui::SameLine();

        const char *verbosityNames[] = {"Verbose", "Debug", "Info", "Warning", "Error", "Fatal"};
        ImVec4 verbosityColors[] = {
            ImVec4(0.5f, 0.5f, 0.5f, 1.0f), // Verbose - Dark Gray
            ImVec4(0.7f, 0.7f, 0.7f, 1.0f), // Debug - Light Gray
            ImVec4(0.0f, 1.0f, 0.0f, 1.0f), // Info - Green
            ImVec4(1.0f, 1.0f, 0.0f, 1.0f), // Warning - Yellow
            ImVec4(1.0f, 0.0f, 0.0f, 1.0f), // Error - Red
            ImVec4(1.0f, 0.0f, 1.0f, 1.0f)  // Fatal - Magenta
        };

        for (int i = 0; i < 6; ++i)
        {
            if (i > 0)
                ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, verbosityColors[i]);
            ImGui::PushStyleColor(ImGuiCol_CheckMark, verbosityColors[i]);
            ImGui::Checkbox(verbosityNames[i], &m_verbosityFilters[i]);
            ImGui::PopStyleColor(2);
        }

        ImGui::Separator();

        // Log message region
        ImGui::BeginChild("LogRegion", ImVec2(0, 0), true,
                          ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

        // Lock messages for reading
        std::lock_guard<std::mutex> lock(m_messagesMutex);

        // Filter and display messages
        bool scrolledToBottom = false;
        for (const auto &msg : m_messages)
        {
            if (!shouldDisplayMessage(msg))
                continue;

            // Format timestamp
            auto timeT = std::chrono::system_clock::to_time_t(msg.timestamp);
            std::tm tmBuf;
#ifdef _WIN32
            localtime_s(&tmBuf, &timeT);
#else
            localtime_r(&timeT, &tmBuf);
#endif
            char timeStr[32];
            std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &tmBuf);

            // Format verbosity name
            const char *verbosityStr = verbosityNames[static_cast<int>(msg.verbosity)];

            // Get color for verbosity
            ImVec4 color = getColorForVerbosity(msg.verbosity);
            ImGui::PushStyleColor(ImGuiCol_Text, color);

            // Format and display message
            ImGui::Text("[%s] [%s] [%s] %s", timeStr, verbosityStr, msg.category.c_str(), msg.message.c_str());

            ImGui::PopStyleColor();

            // Check if we're at the bottom (for auto-scroll)
            if (m_autoScroll && !scrolledToBottom)
            {
                float scrollY = ImGui::GetScrollY();
                float scrollMaxY = ImGui::GetScrollMaxY();
                if (scrollY >= scrollMaxY - 1.0f)
                {
                    scrolledToBottom = true;
                }
            }
        }

        // Auto-scroll to bottom
        if (m_autoScroll && !m_messages.empty() && scrolledToBottom)
        {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
    }

    // Handle window close button
    if (!windowOpen)
    {
        hide();
    }

    ImGui::End();
}

void GUIOutputLog::receiveLogMessage(LogVerbosity verbosity, const std::string &category, const std::string &message)
{
    std::lock_guard<std::mutex> lock(m_messagesMutex);

    m_messages.emplace_back(verbosity, category, message);

    // Limit message count
    if (static_cast<int>(m_messages.size()) > m_maxMessages)
    {
        m_messages.erase(m_messages.begin(), m_messages.begin() + (m_messages.size() - m_maxMessages));
    }
}

void GUIOutputLog::clear()
{
    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.clear();
}

bool GUIOutputLog::shouldDisplayMessage(const LogMessage &msg) const
{
    // Check verbosity filter
    int verbosityIndex = static_cast<int>(msg.verbosity);
    if (verbosityIndex < 0 || verbosityIndex >= 6 || !m_verbosityFilters[verbosityIndex])
    {
        return false;
    }

    // Check category filter
    if (m_categoryFilter[0] != '\0')
    {
        // Case-insensitive search
        std::string filterLower = m_categoryFilter;
        std::string categoryLower = msg.category;
        std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
        std::transform(categoryLower.begin(), categoryLower.end(), categoryLower.begin(), ::tolower);

        if (categoryLower.find(filterLower) == std::string::npos)
        {
            return false;
        }
    }

    return true;
}

ImVec4 GUIOutputLog::getColorForVerbosity(LogVerbosity verbosity) const
{
    switch (verbosity)
    {
    case LogVerbosity::Verbose:
        return ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Dark Gray
    case LogVerbosity::Debug:
        return ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Light Gray
    case LogVerbosity::Info:
        return ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
    case LogVerbosity::Warning:
        return ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
    case LogVerbosity::Error:
        return ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
    case LogVerbosity::Fatal:
        return ImVec4(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
    default:
        return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
    }
}
