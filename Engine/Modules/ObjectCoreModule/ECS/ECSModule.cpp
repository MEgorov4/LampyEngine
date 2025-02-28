#include "ECSModule.h"	
#include <format>
#include <fstream>
#include "../../LoggerModule/Logger.h"
#include "../../ProjectModule/ProjectModule.h"
#include "ECSLuaScriptsSystem.h"
#include "../../ResourceModule/ResourceManager.h"
#include "../../FilesystemModule/FilesystemModule.h"

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
}

void ECSModule::loadInitialWorldState()
{
	m_world.entity("Room").set<Position>({ 0, 0, 0 })
		.set<Rotation>({ 0, 0, 0 })
		.set<Scale>({ 1, 1, 1 })
		.set<Script>({ PM.getInstance().getProjectConfig().getResourcesPath() + "/b/test.lua" })
		.set<MeshComponent>({ "../Resources/Meshes/viking_room.obj"
				, "../Resources/Shaders/GLSL/shader.vert"
				, "../Resources/Shaders/GLSL/shader.frag" });

	m_world.entity("ViewportCamera")
		.set<Position>({ 0, 0, -5.f })
		.set<Rotation>({ 0.0f, 0.0f, 0.0f})
		.set<Camera>({ 75.0f, 16.0f / 9.0f, 0.1f, 100.0f, true });

}

void ECSModule::fillDefaultWorld()
{
	std::string json = m_world.to_json().c_str();

	LOG_INFO(json);
}

void ECSModule::saveCurrentWorld()
{
	std::string strJson = m_world.to_json().c_str();

	std::ofstream file(m_currentWorldFile);
	if (file.is_open())
	{
		file << strJson;
		file.close();
	}
}

void ECSModule::loadWorldFromFile(const std::string& path)
{
	m_currentWorldFile = path;

	std::ifstream file(path);
	std::string strData;
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			strData += line;
		}
		file.close();
	}
	m_world.from_json(strData.c_str());
}

void ECSModule::setCurrentWorldPath(const std::string& path)
{
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
	m_tickEnabled = true;
}

void ECSModule::stopSystems()
{
	LOG_INFO("ECSModule: Stop systems");
	m_tickEnabled = false;

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
	m_world.component<Position>()
		.member("x", &Position::x)
		.member("y", &Position::y)
		.member("z", &Position::z);

	m_world.component<Rotation>()
		.member("x", &Rotation::x)
		.member("y", &Rotation::y)
		.member("z", &Rotation::z);

	m_world.component<Scale>()
		.member("x", &Scale::x)
		.member("y", &Scale::y)
		.member("z", &Scale::z);

	m_world.component<Camera>()
		.member("fov", &Camera::fov)
		.member("aspect", &Camera::aspect)
		.member("farClip", &Camera::farClip)
		.member("nearClip", &Camera::nearClip);

	m_world.component<MeshComponent>()
		.member("meshResourcePath", &MeshComponent::meshResourcePath)
		.member("vertShaderPath", &MeshComponent::vertShaderPath)
		.member("fragShaderPath", &MeshComponent::fragShaderPath);
}

void ECSModule::registerObservers()
{
	m_world.observer<Position>()
		.event(flecs::OnSet)
		.each([=](flecs::entity e, Position& mesh)
			{
				OnComponentsChanged();
			});

	m_world.observer<Rotation>()
		.event(flecs::OnSet)
		.each([=](flecs::entity e, Rotation& mesh)
			{
				OnComponentsChanged();
			});

	m_world.observer<Scale>()
		.event(flecs::OnSet)
		.each([=](flecs::entity e, Scale& mesh)
			{
				OnComponentsChanged();
			});

	m_world.observer<Camera>()
		.event(flecs::OnSet)
		.each([=](flecs::entity e, Camera& mesh)
			{
				OnComponentsChanged();
			});

	m_world.observer<MeshComponent>()
		.event(flecs::OnSet)
		.each([=](flecs::entity e, MeshComponent& mesh)
			{
				mesh.meshResource = ResourceManager::load<RMesh>(mesh.meshResourcePath);
				mesh.vertShaderResource = ResourceManager::load<RShader>(mesh.vertShaderPath);
				mesh.fragShaderResource = ResourceManager::load<RShader>(mesh.fragShaderPath);
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
