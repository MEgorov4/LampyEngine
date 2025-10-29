#include "ECSModule.h"

#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ResourceModule/Mesh.h>
#include <Modules/ResourceModule/Shader.h>
#include <Modules/ResourceModule/Texture.h>
#include <Modules/PhysicsModule/PhysicsModule.h>

#include "Systems/ECSLuaScriptsSystem.h"
#include "Systems/ECSPhysicsSystem.h"
#include "Components/ECSComponents.h"

#include "Additionals/ECSSerializeTypes.h"
namespace ECSModule
{
	void ECSModule::startup()
	{
		m_projectModule = GCXM(ProjectModule::ProjectModule);
		m_resourceManager = GCM(ResourceModule::ResourceManager);
		m_luaScriptModule = GCM(ScriptModule::LuaScriptModule);
		m_physicsModule = GCM(PhysicsModule::PhysicsModule);
		
		LT_LOGI("ECSModule", "Startup");

		m_world = flecs::world();

		openBasicWorld();
	}

	void ECSModule::shutdown()
	{
		LT_LOGI("ECSModule", "Shutdown");
		m_world.reset();
	}

	void ECSModule::openWorld(const std::string& worldData)
	{
		LT_LOGI("ECSModule", "OpenWorld: Data: " + worldData);

		resetWorld();
		m_world.from_json(worldData.c_str());
	}

	void ECSModule::openBasicWorld()
	{
		resetWorld();
		//openWorld(m_filesystemModule->readTextFile("../Resources/World/Templates/basic_world.lworld"));

		auto& viewport_camera = m_world.entity("ViewportCamera")
			.set<PositionComponent>({ 0.f, 2.f, 5.f })      // чуть выше и позади
			.set<RotationComponent>({ -15.f, 0.f, 0.f })    // смотреть на -Z (к центру)
			.set<CameraComponent>({ 75.0f, 16.0f / 9.0f, 0.1f, 100.0f, true });

		m_world.entity("Room")
			.set<PositionComponent>({ 0.f, 0.f, 0.f })
			.set<RotationComponent>({ 0.f, 0.f, 0.f })
			.set<ScaleComponent>({ 1.f, 1.f, 1.f })
			.set<MeshComponent>({
				"../Resources/Meshes/BaseGeometry/cube.obj", "", "",
				""
				});
	}

	void ECSModule::resetWorld()
	{
		m_world.reset();
		m_world.import<flecs::stats>();

		registerTypes();
		registerComponents();
		registerObservers();

		ECSluaScriptsSystem::getInstance().registerSystem(m_world, m_luaScriptModule);
		ECSPhysicsSystem::getInstance().registerSystem(m_world);
	}

	void ECSModule::simulate(bool state)
	{
		if (state)
		{
			m_worldTemp = getCurrentWorldData();
			m_inSimulate = true;
			m_physicsModule->setTickEnabled(true);
			ECSluaScriptsSystem::getInstance().startSystem(m_world);
		}
		else
		{
			ECSluaScriptsSystem::getInstance().stopSystem(m_world);
			m_physicsModule->setTickEnabled(false);
			m_inSimulate = false;
			openWorld(m_worldTemp);
		}
	}

	void ECSModule::ecsTick(float deltaTime)
	{
		if (m_inSimulate)
		{
			if (m_world.progress(deltaTime))
			{
			}
		}
	}

	template <typename T>
	void ECSModule::registerComponent(const std::string& name)
	{
		auto component = m_world.component<T>();
		m_registeredComponents.emplace_back(component.id(), name);
	}

	void ECSModule::registerTypes() {
		m_world.component<std::string>()
			.opaque(flecs::String)
			.serialize(Utils::string_serialize)
			.assign_string(Utils::string_assign_string)
			.assign_null(Utils::string_assign_null);
	}
	void ECSModule::registerComponents()
	{
		m_world.component<PositionComponent>()
			.member("x", &PositionComponent::x)
			.member("y", &PositionComponent::y)
			.member("z", &PositionComponent::z);
		registerComponent<PositionComponent>("PositionComponent");

		m_world.component<RotationComponent>()
			.member("x", &RotationComponent::x)
			.member("y", &RotationComponent::y)
			.member("z", &RotationComponent::z);
		registerComponent<RotationComponent>("RotationComponent");

		m_world.component<ScaleComponent>()
			.member("x", &ScaleComponent::x)
			.member("y", &ScaleComponent::y)
			.member("z", &ScaleComponent::z);
		registerComponent<ScaleComponent>("ScaleComponent");


		m_world.component<ScriptComponent>()
			.member("scriptPath", &ScriptComponent::scriptPath);

		registerComponent<ScriptComponent>("ScriptComponent");

		m_world.component<CameraComponent>()
			.member("fov", &CameraComponent::fov)
			.member("aspect", &CameraComponent::aspect)
			.member("farClip", &CameraComponent::farClip)
			.member("nearClip", &CameraComponent::nearClip);
		registerComponent<CameraComponent>("CameraComponent");

		m_world.component<MeshComponent>()
			.member("meshResourcePath", &MeshComponent::meshResourcePath)
			.member("vertShaderPath", &MeshComponent::vertShaderPath)
			.member("fragShaderPath", &MeshComponent::fragShaderPath)
			.member("texturePath", &MeshComponent::texturePath);
		registerComponent<MeshComponent>("MeshComponent");

		m_world.component<RigidbodyComponent>()
			.member("mass", &RigidbodyComponent::mass);
		registerComponent<RigidbodyComponent>("RigidbodyComponent");

		m_world.component<InvisibleTag>();
	}

	void ECSModule::registerObservers()
	{
		m_world.observer<PositionComponent>()
			.event(flecs::OnSet)
			.each([this](flecs::entity e, PositionComponent& mesh)
				{
					this->OnComponentsChanged();
				});

		m_world.observer<RotationComponent>()
			.event(flecs::OnSet)
			.each([this](flecs::entity e, RotationComponent& mesh)
				{
					this->OnComponentsChanged();
				});

		m_world.observer<ScaleComponent>()
			.event(flecs::OnSet)
			.each([this](flecs::entity e, ScaleComponent& mesh)
				{
					this->OnComponentsChanged();
				});

		m_world.observer<CameraComponent>()
			.event(flecs::OnSet)
			.each([this](flecs::entity e, CameraComponent& mesh)
				{
					this->OnComponentsChanged();
				});

		m_world.observer<DirectionalLightComponent>()
			.event(flecs::OnSet)
			.each([this](flecs::entity e, DirectionalLightComponent& dirLight)
				{
					this->OnComponentsChanged();
				});

		m_world.observer<MeshComponent>()
			.event(flecs::OnSet)
			.each([this](flecs::entity e, MeshComponent& mesh)
				{
					using namespace ResourceModule;
					LT_LOGI("ECSModule", "Reload mesh: " + std::string(e.name().c_str()));

					std::shared_ptr<RMesh> resMesh = m_resourceManager->load<RMesh>(mesh.meshResourcePath);
					if (resMesh && mesh.meshResource != resMesh) mesh.meshResource = resMesh;

					std::shared_ptr<RShader> resVertShader = m_resourceManager->load<RShader>(mesh.vertShaderPath);
					std::shared_ptr<RShader> resFragShader = m_resourceManager->load<RShader>(mesh.fragShaderPath);
					if ((resVertShader && mesh.vertShaderResource != resVertShader) && (resFragShader && mesh.
						fragShaderResource != resFragShader))
					{
						mesh.vertShaderResource = resVertShader;
						mesh.fragShaderResource = resFragShader;
					}

					std::shared_ptr<RTexture> resTexturePath = m_resourceManager->load<RTexture>(mesh.texturePath);
					if (resTexturePath && mesh.textureResource != resTexturePath)
						mesh.textureResource = resTexturePath;

					OnComponentsChanged();
				});

		m_world.observer<MeshComponent>()
			.event(flecs::OnRemove)
			.each([this](flecs::entity e, MeshComponent& mesh)
				{
					LT_LOGI("ECSModule", "Remove mesh: " + std::string(e.name().c_str()));

					using namespace ResourceModule;
					if (mesh.meshResource)
						m_resourceManager->unload<RMesh>(mesh.meshResource.value()->getGUID());

					if (mesh.vertShaderResource)
						m_resourceManager->unload<RShader>(mesh.vertShaderResource.value()->getGUID());
					if (mesh.fragShaderResource)
						m_resourceManager->unload<RShader>(mesh.fragShaderResource.value()->getGUID());

					if (mesh.textureResource)
						m_resourceManager->unload<RShader>(mesh.textureResource.value()->getGUID());

					OnComponentsChanged();
				});
	}
}
