#pragma once
#include <string>
#include <list>
#include "../ImGuiModule/GUIObject.h"

class GUIEditorConsole : GUIObject
{
public:
	GUIEditorConsole();
	~GUIEditorConsole() override = default;

	void render() override;

private:
	void processCommand(const std::string& command);
};
