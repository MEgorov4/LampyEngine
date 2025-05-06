#pragma once

#include <memory>

class BulletDebugDrawer;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

class btVector3;
class btTransform;

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

	void enableDebugDraw(bool newFlag);
private:
	PhysicsModule();

	void setupWorldProperties();

	void drawDebugBox(btVector3 center, btVector3 halfExtents, const btTransform& worldTransform);

	std::unique_ptr<BulletDebugDrawer> m_debugDrawer;

	std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
	std::unique_ptr<btCollisionDispatcher> m_dispatcher;
	std::unique_ptr<btBroadphaseInterface> m_broadphase;
	std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
	std::unique_ptr<btDiscreteDynamicsWorld> m_physicsWorld;

	bool m_tickEnabled = false;
	bool m_shouldDebugDraw = false;
};