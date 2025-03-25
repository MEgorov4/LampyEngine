#pragma once

#include <memory>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

class PhysicsModule
{
public:
	~PhysicsModule();

	static PhysicsModule& getInstance()
	{
		static PhysicsModule resourceManager;
		return resourceManager;
	}

	void startup();
	void shutDown();

	void tick(float deltaTime);

	void registrateBodies();

	void setTickEnabled(bool tickEnabled);

	void clearPhysicsWorld();
private:
	PhysicsModule();

	void setupWorldProperties();

	std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
	std::unique_ptr<btCollisionDispatcher> m_dispatcher;
	std::unique_ptr<btBroadphaseInterface> m_broadphase;
	std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
	std::unique_ptr<btDiscreteDynamicsWorld> m_physicsWorld;

	bool m_tickEnabled = false;
};