#pragma once 
#include <string>
#include <vector>
#include "../ImGuiModule/GUIObject.h"
class GUIOutputLog : public GUIObject
{
	std::vector<std::string> m_messages;
	int m_subscriberID;
public:
	GUIOutputLog();
	~GUIOutputLog();

	void render() override;

	void receiveLogMessage(const std::string& message);
	void clear();
};