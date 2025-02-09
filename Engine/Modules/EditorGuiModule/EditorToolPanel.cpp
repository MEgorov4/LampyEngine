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
		if (!m_ecsModule.getTickEnabled())
		{
			if (ImGui::Button("Start"))
			{
				m_ecsModule.startSystems();
			}
		}

		ImGui::SameLine();

		if (m_ecsModule.getTickEnabled())
		{
			if (ImGui::Button("Stop"))
			{
				m_ecsModule.stopSystems();
			}
		}
	}
	ImGui::End();
}
