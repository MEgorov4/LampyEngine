#pragma once

#include <optional>
#include <memory>
#include <flecs.h>
#include "../Components/ECSComponents.h"
#include <btBulletDynamicsCommon.h>

struct RigidbodyComponent
{
	std::optional<std::shared_ptr<btRigidBody>> body;
	float mass = 0.01f;
	bool isStatic = false;
};

class ECSPhysicsSystem {
	ECSPhysicsSystem() = default;
	ECSPhysicsSystem(const ECSPhysicsSystem&) = delete;
	ECSPhysicsSystem& operator=(const ECSPhysicsSystem&) = delete;

public:
	static ECSPhysicsSystem& getInstance() {
		static ECSPhysicsSystem system;
		return system;
	}

	void registerSystem(flecs::world& world) {

		world.system<RigidbodyComponent>()
			.kind(flecs::OnUpdate)
			.each([](flecs::entity e, RigidbodyComponent& rb) {
					if (rb.body) {
						btTransform trans;
						rb.body.value()->getMotionState()->getWorldTransform(trans);
						btVector3 pos = trans.getOrigin();

						auto& transform = EnsureTransformComponent(e);
						transform.position = { pos.x(), pos.y(), pos.z() };
						e.modified<TransformComponent>();
					}
				});
	}
};