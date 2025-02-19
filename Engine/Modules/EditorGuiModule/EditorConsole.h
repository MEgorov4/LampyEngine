#pragma once

#include "../ImGuiModule/GUIObject.h"
#include <string>

class GUIEditorConsole : GUIObject
{
public:
	GUIEditorConsole();
	~GUIEditorConsole() override = default;

	void render() override;

private:
	void processCommand(const std::string command);
};
