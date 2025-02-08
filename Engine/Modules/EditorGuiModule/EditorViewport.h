#pragma once

#include "../ImGuiModule/GUIObject.h"
#include "vulkan/vulkan.h"
class GUIEditorViewport : GUIObject
{
	VkDescriptorSet m_offscreenImageDescriptor;
public:
	GUIEditorViewport();
	virtual ~GUIEditorViewport() override = default;

	virtual void render() override;
};
