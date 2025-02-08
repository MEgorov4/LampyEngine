#include "EditorToolPanel.h"
#include <imgui.h>
#include "../ObjectCoreModule/ECS/ECSModule.h"

GUIEditorToolPanel::GUIEditorToolPanel() : GUIObject(), m_ecsModule(ECSModule::getInstance())
{
}

void GUIEditorToolPanel::render()
{
	if (ImGui::Begin("Tool panel", nullptr, 0))
	{
		if (ImGui::Button("PlayButton"))
		{
			m_ecsModule.startSystems();
		}
		ImGui::SameLine();
		if (ImGui::Button("StopButton"))
		{
			m_ecsModule.stopSystems();
		}
	}
	ImGui::End();
}
