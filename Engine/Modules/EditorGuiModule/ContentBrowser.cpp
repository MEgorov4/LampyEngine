#include "ContentBrowser.h"
#include <imgui.h>

GUIContentBrowser::GUIContentBrowser()
{
}

void GUIContentBrowser::render()
{
	ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y / 3 * 2));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 3));

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

	ImGui::Begin("ContentBrowser", nullptr, windowFlags);

	ImGui::End();
}
