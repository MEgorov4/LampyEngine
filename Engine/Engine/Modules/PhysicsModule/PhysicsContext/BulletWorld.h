#pragma once

#include <memory>

// Forward declarations
class btDiscreteDynamicsWorld;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btSequentialImpulseConstraintSolver;

namespace PhysicsModule
{
    class BulletWorld
    {
    public:
        BulletWorld();
        ~BulletWorld();

        BulletWorld(const BulletWorld&) = delete;
        BulletWorld& operator=(const BulletWorld&) = delete;

        BulletWorld(BulletWorld&&) = delete;
        BulletWorld& operator=(BulletWorld&&) = delete;

        btDiscreteDynamicsWorld* world;

    private:
        std::unique_ptr<btCollisionDispatcher> m_dispatcher;
        std::unique_ptr<btBroadphaseInterface> m_broadphase;
        std::unique_ptr<btDefaultCollisionConfiguration> m_config;
        std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
    };
}

