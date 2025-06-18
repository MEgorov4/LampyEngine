#pragma once
#include <memory>
#include <optional>
#include <string>

#include <flecs.h>
#include "../../EventModule/Event.h"

#include "ECSComponents.h"
class ECSModule
{
	bool m_tickEnabled = false;
	std::string m_currentWorldFile;
	std::string m_currentWorldData;
	flecs::world m_world;
	std::vector<std::pair<flecs::id_t, std::string>> m_registeredComponents;
public:
	static ECSModule& getInstance()
	{
		static ECSModule ECSModule;
		return ECSModule;
	}

	void startup();
	
	void fillDefaultWorld();
	void loadWorldFromFile(const std::string& path);
	void setCurrentWorldPath(const std::string& path);
	void saveCurrentWorld();
	bool isWorldSetted();
	void clearWorld();

	void startSystems();
	void stopSystems();
	
	bool getTickEnabled() { return m_tickEnabled; }
	flecs::world& getCurrentWorld() { return m_world; };
	std::vector<std::pair<flecs::id_t, std::string>>& getRegisteredComponents() { return m_registeredComponents; };

	void ecsTick(float deltaTime);

	void shutDown();

	Event<> OnLoadInitialWorldState;
	Event<> OnComponentsChanged;

private:
	template<typename T>
	void registerComponent(const std::string& name);
	void registerComponents();
	void registerObservers();
};

