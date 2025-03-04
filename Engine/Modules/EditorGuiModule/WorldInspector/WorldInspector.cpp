#include "WorldInspector.h"
#include <imgui.h>
#include <filesystem>
#include "../../ObjectCoreModule/ECS/ECSModule.h"
#include "../../ProjectModule/ProjectModule.h"
#include "../../LoggerModule/Logger.h"
#include "../../ObjectCoreModule/ECS/ECSLuaScriptsSystem.h"
#include "ComponentsRenderFabric.h"

GUIWorldInspector::GUIWorldInspector() : GUIObject()
		, m_world(ECSModule::getInstance().getCurrentWorld())
{
	ComponentRendererFactory& factory = ComponentRendererFactory::getInstance();
	factory.registerRenderer("PositionComponent", []() {return std::make_unique<PositionRenderer>(); });
	factory.registerRenderer("RotationComponent", []() {return std::make_unique<RotationRenderer>(); });
	factory.registerRenderer("ScaleComponent", []() {return std::make_unique<ScaleRenderer>(); });
	factory.registerRenderer("Script", []() {return std::make_unique<ScriptRenderer>(); });
	factory.registerRenderer("MeshComponent", []() {return std::make_unique<MeshComponentRenderer>(); });
	factory.registerRenderer("CameraComponent", []() {return std::make_unique<CameraRenderer>(); });
	factory.registerRenderer("DirectionalLightComponent", []() {return std::make_unique<DirectionalLightRenderer>(); });
}

void GUIWorldInspector::render()
{

	if (ImGui::Begin("WorldInspector", nullptr, 0))
	{
		ImGui::BeginChild("WorldTree", ImVec2(0, ImGui::GetWindowHeight() * 0.3f), true);
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Tree").x) / 2);

		ImGui::SetWindowFontScale(1.2f);
		ImGui::Text("Tree");
		ImGui::SetWindowFontScale(1);

		ImGui::SetCursorPosX(0);
		ImGui::Separator();
		renderEntityTree();
		renderEntityTreePopup();
		ImGui::EndChild();
		ImGui::BeginChild("ObjectDefault#", ImVec2(0, 0), true);
		renderSelectedEntityDefaults();
		ImGui::EndChild();
	}

	ImGui::End();
}

void GUIWorldInspector::renderEntityTreePopup()
{
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::Button("Add entity"))
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
					m_world.entity(buffer).set<PositionComponent>({0, 0, 0});
				}
			}
			ImGui::EndPopup();
		}
		ImGui::EndPopup();
	}
}

void GUIWorldInspector::renderEntityTree()
{
	auto query = m_world.query<PositionComponent>();
	query.each([&](flecs::entity e, PositionComponent pos)
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
		ImGui::SetWindowFontScale(1.2f);
		ImGui::TextColored(ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f), m_selectedEntity.name());
		ImGui::SetWindowFontScale(1);
		ImGui::SetCursorPosX(0);
		

		auto& factory = ComponentRendererFactory::getInstance();
		
		if (m_selectedEntity.has<PositionComponent>())
		{
			auto renderer = factory.createRenderer("PositionComponent");
			if (renderer) {
				renderer->render(m_selectedEntity);
			}
		}
		
		if (m_selectedEntity.has<RotationComponent>())
		{
			auto renderer = factory.createRenderer("RotationComponent");
			if (renderer) {
				renderer->render(m_selectedEntity);
			}
		}

		if (m_selectedEntity.has<ScaleComponent>())
		{
			auto renderer = factory.createRenderer("ScaleComponent");
			if (renderer) {
				renderer->render(m_selectedEntity);
			}
		}

		if (m_selectedEntity.has<CameraComponent>())
		{
			auto renderer = factory.createRenderer("CameraComponent");
			if (renderer) {
				renderer->render(m_selectedEntity);
			}
		}

		if (m_selectedEntity.has<MeshComponent>())
		{
			auto renderer = factory.createRenderer("MeshComponent");
			if (renderer) {
				renderer->render(m_selectedEntity);
			}
		}

		if (m_selectedEntity.has<Script>())
		{
			auto renderer = factory.createRenderer("Script");
			if (renderer) {
				renderer->render(m_selectedEntity);
			}
		}

		if (m_selectedEntity.has<DirectionalLightComponent>())
		{
			auto renderer = factory.createRenderer("DirectionalLightComponent");
			if (renderer) {
				renderer->render(m_selectedEntity);
			}
		}
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() -  ImGui::CalcTextSize("New component   Remove entity").x) / 2);

		if (ImGui::Button("New component"))
		{
			LOG_INFO("GUIWWorldInspector: Add component button pressed");
		}

		ImGui::SameLine();

		if (ImGui::Button("Remove entity"))
		{
			m_selectedEntity.destruct();
		}
	}
}


