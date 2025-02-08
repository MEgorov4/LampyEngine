#include "OutputLog.h"
#include <imgui.h>

#include "../LoggerModule/Logger.h"

GUIOutputLog::GUIOutputLog() : GUIObject()
{
	m_subscriberID = Logger::Get().OnMessagePushed.subscribe(std::bind(&GUIOutputLog::receiveLogMessage, this, std::placeholders::_1));
}

GUIOutputLog::~GUIOutputLog()
{
	Logger::Get().OnMessagePushed.unsubscribe(m_subscriberID);
}

void GUIOutputLog::render()
{
	ImGui::Begin("Console Log", nullptr, 0);


	if (ImGui::Button("Clear"))
	{
		clear();
	}

	ImGui::Separator();

    ImGui::BeginChild("LogRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& message : m_messages) {
        ImGui::TextUnformatted(message.c_str());
    }

    if (!m_messages.empty()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();

    ImGui::End();
}

void GUIOutputLog::receiveLogMessage(const std::string& message)
{
	m_messages.push_back(message);
}

void GUIOutputLog::clear()
{
	m_messages.clear();
}
