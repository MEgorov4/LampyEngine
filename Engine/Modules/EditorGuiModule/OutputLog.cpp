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
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 3 * 2));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 3));

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::Begin("Console Log", nullptr, windowFlags);


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
