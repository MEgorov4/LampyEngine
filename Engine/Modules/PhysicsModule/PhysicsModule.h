#pragma once

#include <memory>

#include "../../EngineContext/IModule.h"
#include "../../EngineContext/ModuleRegistry.h"

#include "BulletDebugDrawer.h"
namespace ECSModule
{
	class ECSModule;
}

namespace Logger
{
	class Logger;
}



class btVector3;
class btTransform;

class PhysicsModule : public IModule
{
	std::shared_ptr<Logger::Logger> m_logger;
	std::shared_ptr<ECSModule::ECSModule> m_ecsModule;
public:

	void startup(const ModuleRegistry& registry) override;
	void shutdown() override;

	void tick(float deltaTime);

	void registrateBodies();

	void setTickEnabled(bool tickEnabled);

	void clearPhysicsWorld();

	void enableDebugDraw(bool newFlag);
private:
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