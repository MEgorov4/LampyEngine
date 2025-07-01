#include "PhysicsModule.h"

#include <btBulletDynamicsCommon.h>
#include "BulletDebugDrawer.h"
#include "../LoggerModule/Logger.h"
#include "../ObjectCoreModule/ECS/ECSModule.h"
#include "../ObjectCoreModule/ECS/ECSPhysicsSystem.h"


namespace PhysicsModule
{
	void PhysicsModule::startup(const ModuleRegistry& registry)
	{
		m_logger = std::dynamic_pointer_cast<Logger::Logger>(registry.getModule("Logger"));
		m_ecsModule = std::dynamic_pointer_cast<ECSModule::ECSModule>(registry.getModule("ECSModule"));
	
		m_debugDrawer.reset(new BulletDebugDrawer());

		m_collisionConfig.reset(new btDefaultCollisionConfiguration());
		m_dispatcher.reset(new btCollisionDispatcher(m_collisionConfig.get()));
		m_broadphase.reset(new btDbvtBroadphase());
		m_solver.reset(new btSequentialImpulseConstraintSolver());
		m_physicsWorld.reset(new btDiscreteDynamicsWorld(m_dispatcher.get(), m_broadphase.get(), m_solver.get(), m_collisionConfig.get()));

		setupWorldProperties();
		// registrateBodies();
	}

	void PhysicsModule::shutdown()
	{
	
	}

	void PhysicsModule::tick(float deltaTime)
	{
		if (m_tickEnabled)
		{
			m_physicsWorld->stepSimulation(deltaTime, 10, 1.0f / 60.0f);
		}
		if (m_shouldDebugDraw)
		{
			m_physicsWorld->debugDrawWorld();
		}
	}

	void PhysicsModule::setupWorldProperties()
	{
		m_physicsWorld->setGravity(btVector3(0, -9.81f, 0));
		m_physicsWorld->setDebugDrawer(m_debugDrawer.get());
	}

	void PhysicsModule::registrateBodies()
	{
		m_logger->log(Logger::LogVerbosity::Info, "Registration rigid bodies", "Physics Module");

		auto& world = m_ecsModule->getCurrentWorld();

		auto query = world.query<RigidbodyComponent, PositionComponent, RotationComponent, MeshComponent>();

		query.each([&](const flecs::entity& e, RigidbodyComponent& rigidbody, PositionComponent& transform, RotationComponent& rotation, MeshComponent& mesh)
			{
				if (!rigidbody.body.has_value())
				{
					const glm::vec3& AABB = mesh.meshResource.value()->getAABBSize();
					btCollisionShape* shape = new btBoxShape(btVector3(AABB.x, AABB.z, AABB.y) * 0.5f);

					btTransform startTransform;
					startTransform.setIdentity();
					startTransform.setOrigin(btVector3({ transform.x, transform.y, transform.z }));

					btQuaternion rotationQuat;
					rotationQuat.setEulerZYX(rotation.z, rotation.y, rotation.x);  // ������� �� ���� Z, Y, X
					startTransform.setRotation(rotationQuat);

					btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

					btScalar mass = rigidbody.isStatic ? 0.f : rigidbody.mass;
					btVector3 inertia(0, 0, 0);
					shape->calculateLocalInertia(mass, inertia);

					btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
					btRigidBody* body = new btRigidBody(rbInfo);

					rigidbody.body.emplace(body);

					// ��������� ���� � ���
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
					delete body->getMotionState(); // ������� motion state
				}
				delete body->getCollisionShape(); // ������� ������������ �����
			}

			m_physicsWorld->removeCollisionObject(obj);
			// delete obj; // ������� ��� ������
		}
	}

	void PhysicsModule::enableDebugDraw(bool newFlag)
	{
		m_shouldDebugDraw = newFlag;
	}
}