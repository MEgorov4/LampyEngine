#pragma once

#include "../ImGuiModule/GUIObject.h"
class ECSModule;
class GUIEditorToolPanel : public GUIObject
{
	ECSModule& m_ecsModule;
public:
	GUIEditorToolPanel();
	virtual ~GUIEditorToolPanel() override = default;

	virtual void render() override;
};
