#include "ECSModule.h"	
#include <format>
#include <fstream>
#include "../../LoggerModule/Logger.h"
#include "../../ProjectModule/ProjectModule.h"
#include "ECSLuaScriptsSystem.h"

void ECSModule::startup()
{
	LOG_INFO("ECSModule: startup");
	m_world = flecs::world();
	m_world.import<flecs::stats>();

	ProjectConfig& config = ProjectModule::getInstance().getProjectConfig();
	m_currentWorldFile = config.getEditorStartWorld();
	registerComponents();
	loadInitialWorldState();
	ECSluaScriptsSystem::getInstance().registerSystem(m_world);
}

void ECSModule::loadInitialWorldState()
{
	m_world.entity("Bob").set<Position>({ 10, 20, 30 }).set<MeshComponent>({ "../Resources/Meshes/viking_room.obj" });
	m_world.entity("Alice").set<Position>({ 10, 20, 30 }).set<Scale>({1.0f, 1.0f, 1.0f}).set<Rotation>({1,0,0,0});
	m_world.entity("Penis").set<Position>({ 10, 20, 30 });
	// m_world.entity("Penis").set<Script>({ProjectModule::getInstance().getProjectConfig().getResourcesPath() + "/b/test.lua"});
	m_world.entity("Chlen").set<Position>({ 10, 20, 30 });
	// m_world.entity("Chlen").set<Script>({ProjectModule::getInstance().getProjectConfig().getResourcesPath() + "/b/test.lua"});
	m_world.entity("Hero");
	m_world.entity("Hero").set<Position>({ 0, 0, 0 }).set<Camera>({90.f, 0.7f, 0, 100.f, true});
	m_world.entity("Hero").set<Script>({ ProjectModule::getInstance().getProjectConfig().getResourcesPath() + "/b/test.lua"});
	
	std::string json = m_world.to_json().c_str();

	LOG_INFO(json);
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
		.member("w", &Rotation::w)
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
		.member("meshResourcePath", &MeshComponent::meshResourcePath);

}

void ECSModule::ecsTick(float deltaTime)
{
	if (m_tickEnabled)
	{
		if(m_world.progress(deltaTime))
		{
			
		}
	}
}
