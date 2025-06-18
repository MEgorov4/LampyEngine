#include "ECSModule.h"	
#include <format>
#include <fstream>
#include "../../LoggerModule/Logger.h"
#include "../../ProjectModule/ProjectModule.h"
#include "ECSLuaScriptsSystem.h"
#include "ECSPhysicsSystem.h"
#include "../../ResourceModule/ResourceManager.h"
#include "../../FilesystemModule/FilesystemModule.h"
#include "../../PhysicsModule/PhysicsModule.h"
#include <btBulletDynamicsCommon.h>

void ECSModule::startup()
{
	LOG_INFO("ECSModule: startup");
	m_world = flecs::world();
	m_world.import<flecs::stats>();

	m_currentWorldFile = PM.getProjectConfig().getEditorStartWorld();

	registerComponents();
	registerObservers();

	fillDefaultWorld();

	ECSluaScriptsSystem::getInstance().registerSystem(m_world);
	ECSPhysicsSystem::getInstance().registerSystem(m_world);
}

void ECSModule::fillDefaultWorld()
{
	clearWorld();


	if (!isWorldSetted())
	{
		auto& viewport_camera = m_world.entity("ViewportCamera")
			.set<PositionComponent>({ 0.f, 2.f, -1.f })
			.set<RotationComponent>({ 60.f, 0.0f, 0.0f })
			.set<CameraComponent>({ 75.0f, 16.0f / 9.0f, 0.1f, 100.0f, true })
			.add<InvisibleTag>();

		//m_world.entity("floor")
		//	.set<PositionComponent>({ -0.140f, -1.600, 0.000f })
		//	.set<RotationComponent>({ 90.000, 0.f, 0.f })
		//	.set<ScaleComponent>({ 2.080,0.760, 4.240 })
		//	.set<MeshComponent>({ "../Resources/Meshes/BaseGeometry/cube.obj"
		//			, ""
		//			, ""
		//			, "" });

		//m_world.entity("wall")
		//	.set<PositionComponent>({ -2.560, -0.300, 0.f })
		//	.set<RotationComponent>({ -90.000, -180.000, 0.f })
		//	.set<ScaleComponent>({ 0.540, 0.400, 3.500 })
		//	.set<MeshComponent>({ "../Resources/Meshes/BaseGeometry/cube.obj"
		//			, ""
		//			, ""
		//			, "" });

		//m_world.entity("wall1")
		//	.set<PositionComponent>({ -0.290, -1.040,4.650 })
		//	.set<RotationComponent>({ -155.000, 90.000, 0.f })
		//	.set<ScaleComponent>({ 0.970, 1.090, 3.830 })
		//	.set<MeshComponent>({ "../Resources/Meshes/BaseGeometry/cube.obj"
		//			, ""
		//			, ""
		//			, "" });

		//m_world.entity("wall2")
		//	.set<PositionComponent>({ 2.240, -0.470, -0.210 })
		//	.set<RotationComponent>({ -90.000, 0.000, 0.000 })
		//	.set<ScaleComponent>({ 0.510,0.500, 3.680 })
		//	.set<MeshComponent>({ "../Resources/Meshes/BaseGeometry/cube.obj"
		//			, ""
		//			, ""
		//			, "" });

		//m_world.entity("ball")
		//	.set<PositionComponent>({ 0.f, -0.150, 0.f })
		//	.set<RotationComponent>({ 0.f, 0.f, 0.f })
		//	.set<ScaleComponent>({ 0.350,0.350, 0.350 })
		//	.set<MeshComponent>({ "../Resources/Meshes/BaseGeometry/sphere.obj"
		//			, ""
		//			, ""
		//			, "C:/Users/mikhail/Desktop/LampyEngine(Vulkan)/build/Engine/Resources/Textures/Generic/GrayBoxTexture.png" })
		//	.set<ScriptComponent>({""});


		m_world.entity("Room")
			.set<PositionComponent>({ 0.f, 0.f, 0.f })
			.set<RotationComponent>({ 0.f, 0.f, 0.f })
			.set<ScaleComponent>({ 1.f, 1.f, 1.f })
			.set<MeshComponent>({ "../Resources/Meshes/viking_room.obj"
					, ""
					, ""
					, "../Resources/Textures/viking_room.png" });
	}
	else
	{
		loadWorldFromFile(m_currentWorldFile);
	}
}

void ECSModule::saveCurrentWorld()
{
	LOG_DEBUG("ECSModule: saveCurrentWorld");
	std::string strJson = m_world.to_json().c_str();

	if (FS.writeTextFile(m_currentWorldFile, strJson) == FResult::SUCCESS)
		m_currentWorldData = strJson;
	else
		LOG_ERROR("ECSModule: saveCurrentWorld: save world failed");

}

void ECSModule::loadWorldFromFile(const std::string& path)
{
	LOG_DEBUG("ECSModule: loadWorldFromFile path: " + path);
	m_currentWorldFile = path;

	std::string strData = FS.readTextFile(path);

	m_world.from_json(strData.c_str());
	m_currentWorldData = m_currentWorldData = m_world.to_json();
}

void ECSModule::setCurrentWorldPath(const std::string& path)
{
	LOG_DEBUG("ECSModule: setCurrentWorldPath path: " + path);

	m_currentWorldFile = path;
	fillDefaultWorld();
	saveCurrentWorld();
}


bool ECSModule::isWorldSetted()
{
	if (m_currentWorldFile == "default")
	{
		return false;
	}
	else
	{
		return true;
	}
}

void ECSModule::clearWorld()
{
	m_world.each([](flecs::entity& e) {
		if (!e.has<InvisibleTag>())
		{
			e.destruct();
		}
		});
}

void ECSModule::startSystems()
{
	LOG_INFO("ECSModule: Start systems");

	ECSluaScriptsSystem::getInstance().startSystem(m_world);
	m_tickEnabled = true;
	PhysicsModule::getInstance().setTickEnabled(true);
}

void ECSModule::stopSystems()
{
	LOG_INFO("ECSModule: Stop systems");
	m_tickEnabled = false;

	PhysicsModule::getInstance().setTickEnabled(false);
	ECSluaScriptsSystem::getInstance().stopSystem(m_world);

	fillDefaultWorld();
}


void ECSModule::shutDown()
{
	m_world.reset();
}

template<typename T>
void ECSModule::registerComponent(const std::string& name)
{
	auto component = m_world.component<T>();
	m_registeredComponents.emplace_back(component.id(), name);
}

void ECSModule::registerComponents()
{
	m_world.component<PositionComponent>()
		.member("Px", &PositionComponent::x)
		.member("Py", &PositionComponent::y)
		.member("Pz", &PositionComponent::z);
	registerComponent<PositionComponent>("PositionComponent");

	m_world.component<RotationComponent>()
		.member("Rx", &RotationComponent::x)
		.member("Ry", &RotationComponent::y)
		.member("Rz", &RotationComponent::z);
	registerComponent<RotationComponent>("RotationComponent");

	m_world.component<ScaleComponent>()
		.member("Sx", &ScaleComponent::x)
		.member("Sy", &ScaleComponent::y)
		.member("Sz", &ScaleComponent::z);
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
		.each([=](flecs::entity e, PositionComponent& mesh)
			{
				OnComponentsChanged();
			});

	m_world.observer<RotationComponent>()
		.event(flecs::OnSet)
		.each([=](flecs::entity e, RotationComponent& mesh)
			{
				OnComponentsChanged();
			});

	m_world.observer<ScaleComponent>()
		.event(flecs::OnSet)
		.each([=](flecs::entity e, ScaleComponent& mesh)
			{
				OnComponentsChanged();
			});

	m_world.observer<CameraComponent>()
		.event(flecs::OnSet)
		.each([=](flecs::entity e, CameraComponent& mesh)
			{
				OnComponentsChanged();
			});

	m_world.observer<DirectionalLightComponent>()
		.event(flecs::OnSet)
		.each([=](flecs::entity e, DirectionalLightComponent& dirLight)
			{
				OnComponentsChanged();
			});

	m_world.observer<MeshComponent>()
		.event(flecs::OnSet)
		.each([=](flecs::entity e, MeshComponent& mesh)
			{
				std::shared_ptr<RMesh> resMesh = ResourceManager::load<RMesh>(mesh.meshResourcePath);
				if (resMesh && mesh.meshResource != resMesh) mesh.meshResource = resMesh;

				std::shared_ptr<RShader> resVertShader = ResourceManager::load<RShader>(mesh.vertShaderPath);
				std::shared_ptr<RShader> resFragShader = ResourceManager::load<RShader>(mesh.fragShaderPath);
				if ((resVertShader && mesh.vertShaderResource != resVertShader) && (resFragShader && mesh.fragShaderResource != resFragShader))
				{
					mesh.vertShaderResource = resVertShader;
					mesh.fragShaderResource = resFragShader;
				}

				std::shared_ptr<RTexture> resTexturePath = ResourceManager::load<RTexture>(mesh.texturePath);
				if (resTexturePath && mesh.textureResource != resTexturePath)
					mesh.textureResource = resTexturePath;

				OnComponentsChanged();
			});

	m_world.observer<MeshComponent>()
		.event(flecs::OnRemove)
		.each([=](flecs::entity e, MeshComponent& mesh)
			{
				if (mesh.meshResource)
					ResourceManager::unload<RMesh>(mesh.meshResource.value()->getGUID());

				if (mesh.vertShaderResource)
					ResourceManager::unload<RShader>(mesh.vertShaderResource.value()->getGUID());
				if (mesh.fragShaderResource)
					ResourceManager::unload<RShader>(mesh.fragShaderResource.value()->getGUID());

				if (mesh.textureResource)
					ResourceManager::unload<RShader>(mesh.textureResource.value()->getGUID());

				OnComponentsChanged();
			});
}

void ECSModule::ecsTick(float deltaTime)
{
	if (m_tickEnabled)
	{
		if (m_world.progress(deltaTime))
		{

		}
	}
}
