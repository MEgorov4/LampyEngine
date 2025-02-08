#include "WorldInspector.h"
#include <imgui.h>

#include "../ObjectCoreModule/ECS/ECSModule.h"
#include "../LoggerModule/Logger.h"

GUIWorldInspector::GUIWorldInspector() : GUIObject()
		, m_world(ECSModule::getInstance().getCurrentWorld())
{
}

void GUIWorldInspector::render()
{

	if (ImGui::Begin("WorldInspector", nullptr, 0))
	{
		ImGui::Text("Tree");
		ImGui::BeginChild("WorldTree", ImVec2(0, ImGui::GetWindowHeight() * 0.3f), true);
		renderEntityList();
		ImGui::EndChild();
		ImGui::Text("Defaults");
		ImGui::BeginChild("ObjectDefaults", ImVec2(0, 0), true);
		renderSelectedEntityDefaults();
		ImGui::EndChild();
	}

	ImGui::End();
}

void GUIWorldInspector::renderEntityList()
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
		if (const Position* pos = m_selectedEntity.get<Position>())
		{
			float position[3] = { pos->x, pos->y, pos->z };

			if (ImGui::DragFloat3(std::format("Position##{}", m_selectedEntity.id()).c_str(), position))
			{
				m_selectedEntity.set<Position>({ position[0], position[1], position[2] });
			}
		}
	}
}


