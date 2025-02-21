#pragma once 

#include <imgui.h>
#include <filesystem>
#include <flecs.h>
#include "../../ObjectCoreModule/ECS/ECSModule.h"
#include "../../ObjectCoreModule/ECS/ECSLuaScriptsSystem.h"
#include "../../ProjectModule/ProjectModule.h"
#include "../../FilesystemModule/FilesystemModule.h"

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
		ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

		if (ImGui::BeginChildFrame(1, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 3))) {
			if (const Position* pos = entity.get<Position>()) {
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Position").x) / 2);

				ImGui::SetCursorPosX(0);
				ImGui::Separator();

				ImGui::Text("Position");

				ImGui::SameLine();

				float position[3] = { pos->x, pos->y, pos->z };

				if (ImGui::SliderFloat3("##Position", position, -100000, 100000)) {
					entity.set<Position>({ position[0], position[1], position[2] });
				}
			}
		}
		ImGui::EndChildFrame();
	}
};


class RotationRenderer : public IComponentRenderer
{
public:
	void render(flecs::entity& entity) override
	{
		ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);
		
		if (ImGui::BeginChildFrame(1, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 3))) {
			if (auto rot = entity.get<Rotation>()) {
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Rotation").x) / 2);

				ImGui::SetCursorPosX(0);
				ImGui::Separator();

				ImGui::Text("Rotation");

				ImGui::SameLine();
				const glm::vec3 angles = rot->toEuler();
				float rotation[3] = { angles.x, angles.y, angles.y };

				if (ImGui::SliderFloat3("##Rotation", rotation, -360, 360)) {
					const auto newRot = entity.get_mut<Rotation>();
					newRot->fromEuler(glm::vec3(rotation[0], rotation[1], rotation[2]));
				}
			}
		}
		ImGui::EndChildFrame();
	}
};


class ScaleRenderer : public IComponentRenderer
{
public:
	void render(flecs::entity& entity) override
	{
		ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

		if (ImGui::BeginChildFrame(1, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 3))) {
			if (const Scale* scale = entity.get<Scale>()) {
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Scale").x) / 2);

				ImGui::SetCursorPosX(0);
				ImGui::Separator();

				ImGui::Text("Scale");

				ImGui::SameLine();

				float scalev[3] = { scale->x, scale->y, scale->z };

				if (ImGui::SliderFloat3("##Scale", scalev, -1000, 1000)) {
					auto sc = entity.get_mut<Scale>();
					sc->fromGMLVec(glm::vec3(scalev[0], scalev[1], scalev[2]));
				}
			}
		}
		ImGui::EndChildFrame();
	}
};

class MeshComponentRenderer : public IComponentRenderer
{
public:
	void render(flecs::entity& entity) override
	{
		if (ImGui::BeginChildFrame(2, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 4))) {
			if (const MeshComponent* meshComponent = entity.get<MeshComponent>()) {
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("MeshComponent").x) / 2);

				ImGui::SetWindowFontScale(1.2);
				ImGui::Text("MeshComponent");
				ImGui::SetWindowFontScale(1);

				ImGui::SetCursorPosX(0);
				ImGui::Separator();

				ImGui::Text("Path:");
				ImGui::SameLine();

				std::string resPath = ProjectModule::getInstance().getProjectConfig().getResourcesPath();
				ImGui::Text(FS.getFileName(meshComponent->meshResourcePath).c_str());

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath")) {
						std::string droppedPath = static_cast<const char*>(payload->Data);
						if (droppedPath.size() > 4 && droppedPath.substr(droppedPath.size() - 4) == ".obj") {
							MeshComponent* meshComponent = entity.get_mut<MeshComponent>();
							memcpy(droppedPath.data(), meshComponent->meshResourcePath,  sizeof(meshComponent->meshResourcePath));
						}
						LOG_INFO(std::format("Dropped file: {}", droppedPath));
					}
					ImGui::EndDragDropTarget();
				}
			}

		}
		ImGui::EndChildFrame();
	}
};


class ScriptRenderer : public IComponentRenderer {
public:
	void render(flecs::entity& entity) override {
		if (ImGui::BeginChildFrame(2, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 4))) {
			if (const Script* script = entity.get<Script>()) {
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Script").x) / 2);

				ImGui::SetWindowFontScale(1.2);
				ImGui::Text("Script");
				ImGui::SetWindowFontScale(1);

				ImGui::SetCursorPosX(0);
				ImGui::Separator();

				ImGui::Text("Path:");
				ImGui::SameLine();

				std::string resPath = ProjectModule::getInstance().getProjectConfig().getResourcesPath();
				ImGui::Text(FS.getFileName(script->script_path).c_str());

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
			}

		}
		ImGui::EndChildFrame();
	}
};

class CameraRenderer : public IComponentRenderer
{
public:
	void render(flecs::entity& entity) override
	{
		if (ImGui::BeginChildFrame(3, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 2.25))) {
			if (Camera* camera = entity.get_mut<Camera>()) {
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Camera").x) / 2);

				ImGui::SetWindowFontScale(1.2);
				ImGui::Text("Camera");
				ImGui::SetWindowFontScale(1);

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

			}
		}
		ImGui::EndChildFrame();
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
