#pragma once 

#include <imgui.h>
#include <filesystem>
#include <flecs.h>
#include "../../ObjectCoreModule/ECS/ECSModule.h"
#include "../../ObjectCoreModule/ECS/Systems/ECSLuaScriptsSystem.h"
#include "../../ObjectCoreModule/ECS/Systems/ECSPhysicsSystem.h"
#include "../../ProjectModule/ProjectModule.h"
#include "../../FilesystemModule/FilesystemModule.h"
#include <btBulletDynamicsCommon.h>
#include "../../../EngineContext/CoreGlobal.h"

class IComponentRenderer
{
protected:
	ProjectModule::ProjectModule* m_projectModule;
	FilesystemModule::FilesystemModule* m_filesystemModule;
	
public:
	IComponentRenderer() 
		: m_projectModule(GCXM(ProjectModule::ProjectModule))
		, m_filesystemModule(GCM(FilesystemModule::FilesystemModule))
	
	{}
	
	virtual void render(flecs::entity& entity) = 0;
	virtual ~IComponentRenderer() {}
};

class PositionRenderer : public IComponentRenderer
{
public:
	PositionRenderer()
		: IComponentRenderer()
	{
	}

	void render(flecs::entity& entity) override
	{
		ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

		if (ImGui::BeginChildFrame(1, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5f, ImGui::GetWindowSize().y / 3.f))) {
			if (const PositionComponent* pos = entity.get<PositionComponent>()) {
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("PositionComponent").x) / 2.f);

				ImGui::SetCursorPosX(0);
				ImGui::Separator();

				ImGui::Text("PositionComponent");

				ImGui::SameLine();

				float position[3] = { pos->x, pos->y, pos->z };

				if (ImGui::DragFloat3("##PositionComponent", position, 0.01f)) {
					entity.set<PositionComponent>({ position[0], position[1], position[2] });
				}
			}
		}
		ImGui::EndChildFrame();
	}
};


class RotationRenderer : public IComponentRenderer
{
public:
	RotationRenderer()
		: IComponentRenderer()
	{
	}

	void render(flecs::entity& entity) override
	{
		ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

		if (ImGui::BeginChildFrame(1, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 3))) {
			if (auto rot = entity.get<RotationComponent>()) {
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("RotationComponent").x) / 2);

				ImGui::SetCursorPosX(0);
				ImGui::Separator();

				ImGui::Text("RotationComponent");

				ImGui::SameLine();
				
				float rotation[3] = { rot->x, rot->y, rot->z };

				if (ImGui::DragFloat3("##RotationComponent", rotation, 5.f)) {
					entity.set<RotationComponent>({ rotation[0], rotation[1], rotation[2] });
				}
			}
		}
		ImGui::EndChildFrame();
	}
};


class ScaleRenderer : public IComponentRenderer
{
public:
	ScaleRenderer()
		: IComponentRenderer()
	{
	}

	void render(flecs::entity& entity) override
	{
		ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

		if (ImGui::BeginChildFrame(1, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 3))) {
			if (const ScaleComponent* scale = entity.get<ScaleComponent>()) {
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("ScaleComponent").x) / 2);

				ImGui::SetCursorPosX(0);
				ImGui::Separator();

				ImGui::Text("ScaleComponent");

				ImGui::SameLine();

				float scalev[3] = { scale->x, scale->y, scale->z };

				if (ImGui::DragFloat3("##ScaleComponent", scalev, 0.01f, -100, 100)) {
					auto sc = entity.get_mut<ScaleComponent>();
					sc->fromGMLVec(glm::vec3(scalev[0], scalev[1], scalev[2]));
					entity.modified<ScaleComponent>();
				}
			}
		}
		ImGui::EndChildFrame();
	}
};

class MeshComponentRenderer : public IComponentRenderer
{
public:
	MeshComponentRenderer()
		: IComponentRenderer()
	{
	}

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

				ImGui::Text("Mesh path:");
				ImGui::SameLine();

				std::string resPath = m_projectModule->getProjectConfig().getResourcesPath();
				if (!m_filesystemModule->getFileName(meshComponent->meshResourcePath).empty())
					ImGui::Text(m_filesystemModule->getFileName(meshComponent->meshResourcePath).c_str());
				else
					ImGui::Text("empty");

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath")) {
						std::string droppedPath = static_cast<const char*>(payload->Data);

						if (droppedPath.size() > 4 && droppedPath.substr(droppedPath.size() - 4) == ".obj") {
							MeshComponent* meshComponentMut = entity.get_mut<MeshComponent>();
							if (meshComponentMut)
							{
								meshComponentMut->meshResourcePath = droppedPath;
							}
							entity.modified<MeshComponent>();

							/*
							LOG_INFO(std::format("Dropped file: {}", droppedPath));
						*/
						}
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::Text("Texture path:");
				ImGui::SameLine();
				ImGui::Text(m_filesystemModule->getFileName(meshComponent->texturePath).size() > 0 ? m_filesystemModule->getFileName(meshComponent->texturePath).c_str() : "empty");
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath")) {
						std::string droppedPath = static_cast<const char*>(payload->Data);

						if (droppedPath.size() > 4 && droppedPath.substr(droppedPath.size() - 4) == ".png") {
							MeshComponent* meshComponentMut = entity.get_mut<MeshComponent>();
							if (meshComponentMut)
							{
								meshComponentMut->texturePath = droppedPath;

								entity.modified<MeshComponent>();
							}
						}
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
	ScriptRenderer()
		: IComponentRenderer()
	{
	}

	void render(flecs::entity& entity) override {
		if (ImGui::BeginChildFrame(3, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 4))) {
			if (const ScriptComponent* script = entity.get<ScriptComponent>()) {
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("ScriptComponent").x) / 2);

				ImGui::SetWindowFontScale(1.2);
				ImGui::Text("ScriptComponent");
				ImGui::SetWindowFontScale(1);

				ImGui::SetCursorPosX(0);
				ImGui::Separator();

				ImGui::Text("Path:");
				ImGui::SameLine();

				std::string resPath = m_projectModule->getProjectConfig().getResourcesPath();
				ImGui::Text(m_filesystemModule->getFileName(script->scriptPath.c_str()).empty() ? "empty" : m_filesystemModule->getFileName(script->scriptPath.c_str()).c_str());

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilePath")) {
						std::string droppedPath = static_cast<const char*>(payload->Data);
						if (droppedPath.size() > 4 && droppedPath.substr(droppedPath.size() - 4) == ".lua") {
							
							entity.set<ScriptComponent>({droppedPath});
						}
					}
					ImGui::EndDragDropTarget();
				}
			}

		}
		ImGui::EndChildFrame();
	}
};

class DirectionalLightRenderer : public IComponentRenderer {
public:
	DirectionalLightRenderer()
		: IComponentRenderer()
	{
	}

	void render(flecs::entity& entity) override {
		if (ImGui::BeginChildFrame(4, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 4))) {
			if (const DirectionalLightComponent* dirLightComponent = entity.get<DirectionalLightComponent>())
			{
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("DirectionalLight").x) / 2);

				ImGui::SetWindowFontScale(1.2);
				ImGui::Text("DirectionalLight");
				ImGui::SetWindowFontScale(1);

				float intencity = dirLightComponent->intencity;
				if (ImGui::DragFloat("##dirLightIntencity", &intencity, 0.01, 0.0f, 1000.f))
				{
					entity.set<DirectionalLightComponent>({ intencity });
				}
			}

		}
		ImGui::EndChildFrame();
	}
};

class CameraRenderer : public IComponentRenderer
{
public:
	CameraRenderer()
		: IComponentRenderer()
	{
	}

	void render(flecs::entity& entity) override
	{
		if (ImGui::BeginChildFrame(3, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 2.25))) {
			if (CameraComponent* camera = entity.get_mut<CameraComponent>()) {
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("CameraComponent").x) / 2);

				ImGui::SetWindowFontScale(1.2);
				ImGui::Text("CameraComponent");
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
				if (ImGui::SliderFloat("##FOV", &fov, 30.0f, 180.0f)) {
					camera->fov = fov;
					entity.modified<CameraComponent>();
				}

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Aspect ratio:");
				ImGui::SameLine(labelWidth);
				if (ImGui::SliderFloat("##ASPECT", &aspect, 0.01f, 1.0f))
				{
					camera->aspect = aspect;
					entity.modified<CameraComponent>();
				}

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Far clip:");
				ImGui::SameLine(labelWidth);
				if (ImGui::SliderFloat("##FAR", &farClip, 10.0f, 10000.0f))
				{
					camera->farClip = farClip;
					entity.modified<CameraComponent>();
				}

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Near clip:");
				ImGui::SameLine(labelWidth);
				if (ImGui::SliderFloat("##NEAR", &nearClip, 0.01f, 10.0f))
				{
					camera->nearClip = nearClip;
					entity.modified<CameraComponent>();
				}

			}
		}
		ImGui::EndChildFrame();
	}
};

class RigidbodyRenderer : public IComponentRenderer
{
public:
	RigidbodyRenderer()
		: IComponentRenderer()
	{
	}

	void render(flecs::entity& entity) override
	{
		ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x);

		if (ImGui::BeginChildFrame(5, ImVec2(ImGui::GetWindowSize().x - ImGui::GetCursorStartPos().x * 3.5, ImGui::GetWindowSize().y / 3)))
		{
			if (RigidbodyComponent* body = entity.get_mut<RigidbodyComponent>())
			{
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("RigidbodyComponent").x) / 2);

				ImGui::SetWindowFontScale(1.2);
				ImGui::Text("RigidbodyComponent");
				ImGui::SetWindowFontScale(1);

				ImGui::SetCursorPosX(0);
				ImGui::Separator();



				float mass = body->mass;
				bool isStatic = body->isStatic;

				ImGui::Text("static");
				ImGui::SameLine();

				if (ImGui::Checkbox("##static", &isStatic))
				{
					body->isStatic = isStatic;
				}

				ImGui::Text("mass");
				ImGui::SameLine();

				ImGui::BeginDisabled(isStatic);
				if (ImGui::DragFloat("##mass", &mass, 0.1f, 0.f, 100000000.f))
				{
					body->mass = mass;
				}
				ImGui::EndDisabled();
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
