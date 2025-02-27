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
	flecs::world m_world;
public:
	static ECSModule& getInstance()
	{
		static ECSModule ECSModule;
		return ECSModule;
	}

	void startup();
	
	void loadInitialWorldState();

	void fillDefaultWorld();
	void loadWorldFromFile(const std::string& path);
	void setCurrentWorldPath(const std::string& path);
	void saveCurrentWorld();
	bool isWorldSetted();
	void clearWorld();

	void startSystems();
	void stopSystems();
	
	bool getTickEnabled() { return m_tickEnabled; }

	flecs::world& getCurrentWorld();

	void ecsTick(float deltaTime);

	void shutDown();

	Event<> OnLoadInitialWorldState;

private:
	void registerComponents();
	void registerObservers();
};