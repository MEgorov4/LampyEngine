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

	ProjectConfig& config = ProjectModule::getInstance().getProjectConfig();
	m_currentWorldFile = config.getEditorStartWorld();

	registerComponents();
	registerObservers();

	loadInitialWorldState();


	ECSluaScriptsSystem::getInstance().registerSystem(m_world);
	ECSPhysicsSystem::getInstance().registerSystem(m_world);

}

void ECSModule::loadInitialWorldState()
{
	if (isWorldSetted())
	{
		loadWorldFromFile(m_currentWorldFile);
	}
	else
	{
		fillDefaultWorld();
	}
}

void ECSModule::fillDefaultWorld()
{
	m_world.entity("Room").set<PositionComponent>({ 0.f, 0.f, 0.f })
		.set<RotationComponent>({ 0.f, 0.f, 0.f })
		.set<ScaleComponent>({ 1.f, 1.f, 1.f })
		// .set<Script>({ PM.getInstance().getProjectConfig().getResourcesPath() + "/b/test.lua" })
		.set<MeshComponent>({ "../Resources/Meshes/viking_room.obj"
				, "../Resources/Shaders/GLSL/shader.vert"
				, "../Resources/Shaders/GLSL/shader.frag"
				, "../Resources/Textures/viking_room.png" });

	m_world.entity("DirectionalLight")
		.set<PositionComponent>({ 0.f, 2.f, 0.f })
		.set<RotationComponent>({ 0.f, 0.f, 0.f })
		.set<DirectionalLightComponent>({ 10 });

	m_world.entity("ViewportCamera")
		.set<PositionComponent>({ 0.f, 0.f, -5.f })
		.set<RotationComponent>({ 0.0f, 0.0f, 0.0f })
		.set<CameraComponent>({ 75.0f, 16.0f / 9.0f, 0.1f, 100.0f, true });
}

void ECSModule::saveCurrentWorld()
{
	LOG_DEBUG("ECSModule: saveCurrentWorld");
	std::string strJson = m_world.to_json().c_str();

	FS.writeTextFile(m_currentWorldFile, strJson);

	m_currentWorldData = strJson;
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
	clearWorld();
	loadWorldFromFile(m_currentWorldFile);
}


bool ECSModule::isWorldSetted()
{
	return m_currentWorldFile != "default";
}

void ECSModule::clearWorld()
{
	m_world.each([](flecs::entity& e) {e.destruct(); });
}

void ECSModule::startSystems()
{
	LOG_INFO("ECSModule: Start systems");

	ECSluaScriptsSystem::getInstance().startSystem(m_world);
	PhysicsModule::getInstance().registrateBodies();
	m_tickEnabled = true;
	PhysicsModule::getInstance().setTickEnabled(true);

	/*auto& world = ECSModule::getInstance().getCurrentWorld();

	auto query = world.query<MeshComponent>();

	query.each([&](const flecs::entity& e, MeshComponent& mesh)
		{
			ResourceManager::unload<RMesh>(mesh.meshResourcePath);
			ResourceManager::unload<RShader>(mesh.vertShaderPath);
			ResourceManager::unload<RShader>(mesh.fragShaderPath);
			ResourceManager::unload<RTexture>(mesh.texturePath);
		});*/
}

void ECSModule::stopSystems()
{
	LOG_INFO("ECSModule: Stop systems");
	m_tickEnabled = false;
	PhysicsModule::getInstance().setTickEnabled(false);

	PhysicsModule::getInstance().clearPhysicsWorld();

	ECSluaScriptsSystem::getInstance().stopSystem(m_world);
	clearWorld();
	loadInitialWorldState();
}

flecs::world& ECSModule::getCurrentWorld()
{
	return m_world;
}

void ECSModule::shutDown()
{
	m_world.reset();
}

void ECSModule::registerComponents()
{
	m_world.component<PositionComponent>()
		.member("x", &PositionComponent::x)
		.member("y", &PositionComponent::y)
		.member("z", &PositionComponent::z);

	m_world.component<RotationComponent>()
		.member("x", &RotationComponent::x)
		.member("y", &RotationComponent::y)
		.member("z", &RotationComponent::z);

	m_world.component<ScaleComponent>()
		.member("x", &ScaleComponent::x)
		.member("y", &ScaleComponent::y)
		.member("z", &ScaleComponent::z);

	m_world.component<CameraComponent>()
		.member("fov", &CameraComponent::fov)
		.member("aspect", &CameraComponent::aspect)
		.member("farClip", &CameraComponent::farClip)
		.member("nearClip", &CameraComponent::nearClip);

	m_world.component<MeshComponent>()
		.member("meshResourcePath", &MeshComponent::meshResourcePath)
		.member("vertShaderPath", &MeshComponent::vertShaderPath)
		.member("fragShaderPath", &MeshComponent::fragShaderPath)
		.member("texturePath", &MeshComponent::texturePath);

	m_world.component<RigidbodyComponent>()
		.member("mass", &RigidbodyComponent::mass);
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
}

void ECSModule::ecsTick(float deltaTime)
{
	if (m_tickEnabled && m_world.progress(deltaTime))
	{

	}
}
