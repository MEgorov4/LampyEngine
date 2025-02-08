#include "EditorViewport.h"
#include <imgui.h>
#include "../RenderModule/RenderModule.h"
GUIEditorViewport::GUIEditorViewport() : GUIObject(), m_offscreenImageDescriptor(RenderModule::getInstance().getRenderer()->getVulkanOffscreenImageView())
{
}

void GUIEditorViewport::render()
{
	if (ImGui::Begin("Viewport", 0, 0));
	if (m_offscreenImageDescriptor)
	{
			ImGui::Image(m_offscreenImageDescriptor, ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 35));
	}
	ImGui::End();
}
