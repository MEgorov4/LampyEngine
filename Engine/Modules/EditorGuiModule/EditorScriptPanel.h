#pragma once
#include <string>
#include <list>
#include "../ImGuiModule/GUIObject.h"

class GUIEditorScriptPanel : GUIObject
{

public:
	GUIEditorScriptPanel();
	~GUIEditorScriptPanel() override = default;

	void render() override;
};
