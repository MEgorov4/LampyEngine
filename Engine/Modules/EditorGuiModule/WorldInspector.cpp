#include "WorldInspector.h"
#include <imgui.h>

#include "../ObjectCoreModule/ECS/ECSModule.h"
#include "../LoggerModule/Logger.h"
#include "../ObjectCoreModule/ECS/ECSLuaScriptsSystem.h"

GUIWorldInspector::GUIWorldInspector() : GUIObject()
		, m_world(ECSModule::getInstance().getCurrentWorld())
{
}

void GUIWorldInspector::render()
{

	if (ImGui::Begin("WorldInspector##", nullptr, 0))
	{
		ImGui::BeginChild("WorldTree##", ImVec2(0, ImGui::GetWindowHeight() * 0.3f), true);
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Tree").x) / 2);
		ImGui::Text("Tree##");
		ImGui::SetCursorPosX(0);
		ImGui::Separator();
		renderEntityTree();
		renderEntityTreePopup();
		ImGui::EndChild();
		ImGui::BeginChild("ObjectDefaults##", ImVec2(0, 0), true);
		renderSelectedEntityDefaults();
		ImGui::EndChild();
	}

	ImGui::End();
}

void GUIWorldInspector::renderEntityTreePopup()
{
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::Button("Add entity##1"))
		{
			ImGui::OpenPopup("CreateEntityPopup##2");
		}
		if (ImGui::BeginPopup("CreateEntityPopup##2"))
		{
			static char buffer[128] = "";
			ImGui::InputText("##EntityName", buffer, sizeof(buffer));

			ImGui::SameLine();
			std::string strBuffer = buffer;
			if (ImGui::Button("CreateEntity##5"))
			{
				if (strBuffer.size() > 0)
				{
					m_world.entity(buffer).set<Position>({0, 0, 0});
				}
			}
			ImGui::EndPopup();
		}
		ImGui::EndPopup();
	}
}

void GUIWorldInspector::renderEntityTree()
{
	auto query = m_world.query<Position>();
	query.each([&](flecs::entity e, Position pos)
		{
			if (ImGui::Selectable(std::format("{}##{}", e.name().c_str(), e.id()).c_str()))
			{
				m_selectedEntity = e;
			}
		});
}

void GUIWorldInspector::renderSelectedEntityDefaults()
{
	if (m_selectedEntity.is_valid())
	{
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(m_selectedEntity.name()).x) / 2);
		ImGui::TextColored(ImVec4(0,1,0,1), m_selectedEntity.name());
		ImGui::SetCursorPosX(0);
		if (const Position* pos = m_selectedEntity.get<Position>())
		{
			ImGui::Separator();
			ImGui::Text("Position");
			ImGui::SameLine();
			float position[3] = { pos->x, pos->y, pos->z };

			if (ImGui::DragFloat3(std::format("##{}", m_selectedEntity.id()).c_str(), position))
			{
				m_selectedEntity.set<Position>({ position[0], position[1], position[2] });
			}
			ImGui::Separator();
		}
		if (const Script* script = m_selectedEntity.get<Script>())
		{
			ImGui::Separator();
			ImGui::Text("Script");
			ImGui::SameLine();
	
			ImGui::Text(script->script_path.c_str());
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath"))
				{
					std::string droppedPath = static_cast<const char*>(payload->Data);
					if (droppedPath.size() > 4 && droppedPath.substr(droppedPath.size() - 4) == ".lua")
					{
						m_selectedEntity.set<Script>({droppedPath});
					}
					LOG_INFO(std::format("Dropped file: {}", droppedPath));
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::Separator();
		}
	
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() -  ImGui::CalcTextSize("New component").x) / 2);
		if (ImGui::Button("New component"))
		{
			LOG_INFO("GUIWWorldInspector: Add component button pressed");
		}
	}
}


