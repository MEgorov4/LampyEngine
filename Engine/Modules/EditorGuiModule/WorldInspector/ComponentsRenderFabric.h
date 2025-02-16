#pragma once 

#include <imgui.h>
#include <filesystem>
#include <flecs.h>
#include "../../ObjectCoreModule/ECS/ECSModule.h"
#include "../../ObjectCoreModule/ECS/ECSLuaScriptsSystem.h"
#include "../../ProjectModule/ProjectModule.h"

class IComponentRenderer
{
public:
	virtual void render(flecs::entity& entity) = 0;
	virtual ~IComponentRenderer() {}
};

class PositionRenderer : public IComponentRenderer
{
public:
	void render(flecs::entity& entity) override
	{
		if (const Position* pos = entity.get<Position>()) {
			ImGui::Separator();
			ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Position").x) / 2);
			ImGui::Text("Position");
			ImGui::SetCursorPosX(0);
			ImGui::Separator();

			ImGui::Text("Position");
			ImGui::SameLine();

			float position[3] = { pos->x, pos->y, pos->z };

			if (ImGui::SliderFloat3("##", position, -100000, 100000)) {
				entity.set<Position>({ position[0], position[1], position[2] });
			}
			ImGui::Separator();
		}
	}
};

class ScriptRenderer : public IComponentRenderer {
public:
	void render(flecs::entity& entity) override {
		if (const Script* script = entity.get<Script>()) {
			ImGui::Separator();

			ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Script").x) / 2);
			ImGui::Text("Script");
			ImGui::SetCursorPosX(0);
			ImGui::Separator();

			ImGui::Text("Path:");
			ImGui::SameLine();

			std::string resPath = ProjectModule::getInstance().getProjectConfig().getResourcesPath();
			ImGui::Text(std::filesystem::relative(
				std::filesystem::path(script->script_path),
				std::filesystem::path(resPath)
			).string().c_str());

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath")) {
					std::string droppedPath = static_cast<const char*>(payload->Data);
					if (droppedPath.size() > 4 && droppedPath.substr(droppedPath.size() - 4) == ".lua") {
						entity.set<Script>({ droppedPath });
					}
					LOG_INFO(std::format("Dropped file: {}", droppedPath));
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::Separator();
		}
	}
};

class CameraRenderer : public IComponentRenderer 
{
public:
	void render(flecs::entity& entity) override 
	{
		if (Camera* camera = entity.get_mut<Camera>()) {
			ImGui::Separator();
			ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Camera").x) / 2);
			ImGui::Text("Camera");
			ImGui::SetCursorPosX(0);
			ImGui::Separator();

			float fov = camera->fov;
			float aspect = camera->aspect;
			float farClip = camera->farClip;
			float nearClip = camera->nearClip;

			const float labelWidth = 110.0f; 


			ImGui::AlignTextToFramePadding();
			ImGui::Text("Field of view:");
			ImGui::SameLine(labelWidth);
			if (ImGui::SliderFloat("##FOV", &fov, 30.0f, 180.0f)) camera->fov = fov;

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Aspect ratio:");
			ImGui::SameLine(labelWidth);
			if (ImGui::SliderFloat("##ASPECT", &aspect, 0.01f, 1.0f)) camera->aspect = aspect;

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Far clip:");
			ImGui::SameLine(labelWidth);
			if (ImGui::SliderFloat("##FAR", &farClip, 10.0f, 10000.0f)) camera->farClip = farClip;

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Near clip:");
			ImGui::SameLine(labelWidth);
			if (ImGui::SliderFloat("##NEAR", &nearClip, 0.01f, 10.0f)) camera->nearClip = nearClip;

			ImGui::Separator();
		}
	}
};

class ComponentRendererFactory
{
public:
	using RendererCreator = std::function<std::unique_ptr<IComponentRenderer>()>;

	static ComponentRendererFactory& getInstance() {
		static ComponentRendererFactory instance;
		return instance;
	}

	void registerRenderer(const std::string& componentType, RendererCreator creator)
	{
		registry[componentType] = creator;
	}

	std::unique_ptr<IComponentRenderer> createRenderer(const std::string& componentType)
	{
		if (registry.find(componentType) != registry.end())
		{
			return registry[componentType]();
		}
		return nullptr;
	}
private:
	std::unordered_map<std::string, RendererCreator> registry;
};
