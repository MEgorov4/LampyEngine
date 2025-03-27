#include "PhysicsModule.h"

#include <btBulletDynamicsCommon.h>
#include "../LoggerModule/Logger.h"
#include "../ObjectCoreModule/ECS/ECSModule.h"
#include "../ObjectCoreModule/ECS/ECSPhysicsSystem.h"

PhysicsModule::PhysicsModule()
{

}

PhysicsModule::~PhysicsModule()
{

}

void PhysicsModule::startup()
{
	LOG_INFO("PhysicsModule: Startup");

	m_collisionConfig.reset(new btDefaultCollisionConfiguration());
	m_dispatcher.reset(new btCollisionDispatcher(m_collisionConfig.get()));
	m_broadphase.reset(new btDbvtBroadphase());
	m_solver.reset(new btSequentialImpulseConstraintSolver());
	m_physicsWorld.reset(new btDiscreteDynamicsWorld(m_dispatcher.get(), m_broadphase.get(), m_solver.get(), m_collisionConfig.get()));

	setupWorldProperties();
	// registrateBodies();
}

void PhysicsModule::shutDown()
{
	LOG_INFO("PhysicsModule: Shut down");


}

void PhysicsModule::tick(float deltaTime)
{
	if (m_tickEnabled)
	{
		// LOG_INFO(std::format("Physics update: {}", deltaTime));
		m_physicsWorld->stepSimulation(deltaTime, 10, 1.0f / 60.0f);
	}
}

void PhysicsModule::setupWorldProperties()
{
	m_physicsWorld->setGravity(btVector3(0, -0.51f, 0));
}

void PhysicsModule::registrateBodies()
{
	LOG_INFO("Physics Module: Rigistrate Rigidbodies");

	auto& world = ECSModule::getInstance().getCurrentWorld();

	auto query = world.query<RigidbodyComponent, PositionComponent, MeshComponent>();

	query.each([&](const flecs::entity& e, RigidbodyComponent& rigidbody, PositionComponent& transform, MeshComponent& mesh)
		{
			if (!rigidbody.body.has_value())
			{
				const glm::vec3& AABB = mesh.meshResource.value()->getAABBSize();
				btCollisionShape* shape = new btBoxShape(btVector3(AABB.x, AABB.z, AABB.y) * 0.5f);

				btTransform startTransform;
				startTransform.setIdentity();
				startTransform.setOrigin(btVector3({ transform.x, transform.y, transform.z }));

				btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

				btScalar mass = rigidbody.mass;
				btVector3 inertia(0, 0, 0);
				shape->calculateLocalInertia(mass, inertia);

				btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
				btRigidBody* body = new btRigidBody(rbInfo);

				rigidbody.body.emplace(body);

				// Добавляем тело в мир
				m_physicsWorld->addRigidBody(body);
			}
		});
}

void PhysicsModule::setTickEnabled(bool tickEnabled)
{
	m_tickEnabled = tickEnabled;
}

void PhysicsModule::clearPhysicsWorld()
{
	int numBodies = m_physicsWorld->getNumCollisionObjects();
	for (int i = numBodies - 1; i >= 0; --i)
	{
		btCollisionObject* obj = m_physicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);

		if (body)
		{
			if (body->getMotionState())
			{
				delete body->getMotionState(); // Удаляем motion state
			}
			delete body->getCollisionShape(); // Удаляем коллизионную форму
		}

		m_physicsWorld->removeCollisionObject(obj);
		// delete obj; // Удаляем сам объект
	}
}
