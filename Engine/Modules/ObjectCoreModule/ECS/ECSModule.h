#pragma once
#include <memory>
#include <flecs.h>

struct Position
{
	float x;
	float y;
	float z;
};

class ECSModule
{
	bool m_tickEnabled = false;
	flecs::world m_world;
public:
	static ECSModule& getInstance()
	{
		static ECSModule ECSModule;
		return ECSModule;
	}

	void startUp();
	
	void loadInitialWorldState();
	void clearWorld();
	void startSystems();
	void stopSystems();

	flecs::world& getCurrentWorld();

	void ecsTick(float deltaTime);

	void shutDown();
};