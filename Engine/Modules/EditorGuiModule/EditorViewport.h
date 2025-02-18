#pragma once

#include "../ImGuiModule/GUIObject.h"
class GUIEditorViewport : GUIObject
{
	void* m_offscreenImageDescriptor;
public:
	GUIEditorViewport();
	virtual ~GUIEditorViewport() override = default;

	virtual void render() override;
};
