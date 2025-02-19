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

	loadInitialWorldState();
	ECSluaScriptsSystem::getInstance().registerSystem(m_world);

	m_world.component<Script>();
}

void ECSModule::loadInitialWorldState()
{
	m_world.entity("Bob").set<Position>({ 10, 20, 30 }).set<MeshComponent>({ "../Resources/Meshes/viking_room.obj" });
	m_world.entity("Alice").set<Position>({ 10, 20, 30 });
	auto& c = m_world.entity("Penis").set<Position>({ 10, 20, 30 });
	// m_world.entity("Penis").set<Script>({ProjectModule::getInstance().getProjectConfig().getResourcesPath() + "/b/test.lua"});
	m_world.entity("Chlen").set<Position>({ 10, 20, 30 });
	// m_world.entity("Chlen").set<Script>({ProjectModule::getInstance().getProjectConfig().getResourcesPath() + "/b/test.lua"});
	flecs::entity entity = m_world.entity("Hero");
	m_world.entity("Hero").set<Position>({ 0, 0, 0 }).set<Camera>({90, 0.7, 0, 100, true});
	m_world.entity("Hero").set<Script>({ ProjectModule::getInstance().getProjectConfig().getResourcesPath() + "/b/test.lua" }); 
	
	
	//if (m_currentWorldFile == "default")
	//{
	//	fillDefaultWorld();
	//}
	//else
	//{
	//	loadWorldFromFile(m_currentWorldFile);
	//}
	std::string json = c.to_json().c_str();

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

void ECSModule::ecsTick(float deltaTime)
{
	if (m_tickEnabled)
	{
		m_world.progress(deltaTime);
	}
}
