#include "EditorToolPanel.h"

GUIEditorToolPanel::GUIEditorToolPanel() : GUIObject()
{

}

void GUIEditorToolPanel::render()
{
	ImGui::SetNextWindowPos(ImVec2(0, 18));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 18));

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar;
	if (ImGui::Begin("Tool panel", nullptr, windowFlags))
	{
		ImGui::Text("hello");
	}
	ImGui::End();
}
