#pragma once

#include "../ImGuiModule/GUIObject.h"

class GUIEditorToolPanel : public GUIObject
{
public:
	GUIEditorToolPanel();
	virtual ~GUIEditorToolPanel() override = default;

	virtual void render() override;
};
