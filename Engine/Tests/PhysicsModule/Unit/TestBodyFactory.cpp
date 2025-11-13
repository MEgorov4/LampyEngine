#include <gtest/gtest.h>
#include <Modules/PhysicsModule/Factory/BodyFactory.h>
#include <Modules/PhysicsModule/Utils/PhysicsTypes.h>
#include <btBulletDynamicsCommon.h>

using namespace PhysicsModule;

class BodyFactoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create minimal Bullet world for testing
        m_config = std::make_unique<btDefaultCollisionConfiguration>();
        m_dispatcher = std::make_unique<btCollisionDispatcher>(m_config.get());
        m_broadphase = std::make_unique<btDbvtBroadphase>();
        m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        m_world = std::make_unique<btDiscreteDynamicsWorld>(
            m_dispatcher.get(),
            m_broadphase.get(),
            m_solver.get(),
            m_config.get()
        );
    }
    
    void TearDown() override
    {
        // Cleanup bodies
        if (m_world)
        {
            int numObjects = m_world->getNumCollisionObjects();
            for (int i = numObjects - 1; i >= 0; --i)
            {
                btCollisionObject* obj = m_world->getCollisionObjectArray()[i];
                btRigidBody* body = btRigidBody::upcast(obj);
                if (body && body->getMotionState())
                {
                    delete body->getMotionState();
                }
                if (body && body->getCollisionShape())
                {
                    delete body->getCollisionShape();
                }
                m_world->removeCollisionObject(obj);
                delete body;
            }
        }
        
        m_world.reset();
        m_solver.reset();
        m_broadphase.reset();
        m_dispatcher.reset();
        m_config.reset();
    }
    
    std::unique_ptr<btDefaultCollisionConfiguration> m_config;
    std::unique_ptr<btCollisionDispatcher> m_dispatcher;
    std::unique_ptr<btBroadphaseInterface> m_broadphase;
    std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
    std::unique_ptr<btDiscreteDynamicsWorld> m_world;
};

// ============================================================================
// Static Body Tests
// ============================================================================

TEST_F(BodyFactoryTest, CreateStaticBody)
{
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Static;
    desc.mass = 0.0f; // Static bodies have zero mass
    desc.position = glm::vec3(0.0f, 0.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(1.0f, 1.0f, 1.0f);
    
    btRigidBody* body = BodyFactory::Create(desc, m_world.get());
    ASSERT_NE(body, nullptr);
    
    EXPECT_TRUE(body->isStaticObject());
    EXPECT_FLOAT_EQ(body->getMass(), 0.0f);
    EXPECT_EQ(m_world->getNumCollisionObjects(), 1);
    
    delete body->getMotionState();
    delete body->getCollisionShape();
    m_world->removeCollisionObject(body);
    delete body;
}

// ============================================================================
// Dynamic Body Tests
// ============================================================================

TEST_F(BodyFactoryTest, CreateDynamicBody)
{
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Dynamic;
    desc.mass = 1.0f;
    desc.position = glm::vec3(0.0f, 10.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(1.0f, 1.0f, 1.0f);
    
    btRigidBody* body = BodyFactory::Create(desc, m_world.get());
    ASSERT_NE(body, nullptr);
    
    EXPECT_FALSE(body->isStaticObject());
    EXPECT_FALSE(body->isKinematicObject());
    EXPECT_FLOAT_EQ(body->getMass(), 1.0f);
    EXPECT_GT(body->getInvMass(), 0.0f);
    EXPECT_EQ(m_world->getNumCollisionObjects(), 1);
    
    delete body->getMotionState();
    delete body->getCollisionShape();
    m_world->removeCollisionObject(body);
    delete body;
}

TEST_F(BodyFactoryTest, CreateDynamicBodyWithCustomMass)
{
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Dynamic;
    desc.mass = 5.0f;
    desc.position = glm::vec3(0.0f, 0.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Sphere;
    desc.shape.radius = 1.0f;
    
    btRigidBody* body = BodyFactory::Create(desc, m_world.get());
    ASSERT_NE(body, nullptr);
    
    EXPECT_FLOAT_EQ(body->getMass(), 5.0f);
    
    delete body->getMotionState();
    delete body->getCollisionShape();
    m_world->removeCollisionObject(body);
    delete body;
}

// ============================================================================
// Kinematic Body Tests
// ============================================================================

TEST_F(BodyFactoryTest, CreateKinematicBody)
{
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Kinematic;
    desc.mass = 0.0f; // Kinematic bodies typically have zero mass
    desc.position = glm::vec3(0.0f, 0.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(1.0f, 1.0f, 1.0f);
    
    btRigidBody* body = BodyFactory::Create(desc, m_world.get());
    ASSERT_NE(body, nullptr);
    
    EXPECT_TRUE(body->isKinematicObject());
    EXPECT_FALSE(body->isStaticObject());
    EXPECT_EQ(body->getActivationState(), DISABLE_DEACTIVATION);
    EXPECT_EQ(m_world->getNumCollisionObjects(), 1);
    
    delete body->getMotionState();
    delete body->getCollisionShape();
    m_world->removeCollisionObject(body);
    delete body;
}

// ============================================================================
// Transform Tests
// ============================================================================

TEST_F(BodyFactoryTest, CreateBodyWithCustomTransform)
{
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Dynamic;
    desc.mass = 1.0f;
    desc.position = glm::vec3(10.0f, 20.0f, 30.0f);
    desc.rotation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    desc.shape.type = PhysicsShapeType::Box;
    desc.shape.size = glm::vec3(1.0f, 1.0f, 1.0f);
    
    btRigidBody* body = BodyFactory::Create(desc, m_world.get());
    ASSERT_NE(body, nullptr);
    
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    
    btVector3 origin = transform.getOrigin();
    EXPECT_NEAR(origin.x(), 10.0f, 0.01f);
    EXPECT_NEAR(origin.y(), 20.0f, 0.01f);
    EXPECT_NEAR(origin.z(), 30.0f, 0.01f);
    
    delete body->getMotionState();
    delete body->getCollisionShape();
    m_world->removeCollisionObject(body);
    delete body;
}

// ============================================================================
// Shape Tests
// ============================================================================

TEST_F(BodyFactoryTest, CreateBodyWithSphereShape)
{
    RigidBodyDesc desc;
    desc.bodyType = RigidBodyType::Dynamic;
    desc.mass = 1.0f;
    desc.position = glm::vec3(0.0f, 0.0f, 0.0f);
    desc.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    desc.shape.type = PhysicsShapeType::Sphere;
    desc.shape.radius = 2.0f;
    
    btRigidBody* body = BodyFactory::Create(desc, m_world.get());
    ASSERT_NE(body, nullptr);
    
    btCollisionShape* shape = body->getCollisionShape();
    ASSERT_NE(shape, nullptr);
    
    btSphereShape* sphereShape = dynamic_cast<btSphereShape*>(shape);
    ASSERT_NE(sphereShape, nullptr);
    EXPECT_NEAR(sphereShape->getRadius(), 2.0f, 0.01f);
    
    delete body->getMotionState();
    delete body->getCollisionShape();
    m_world->removeCollisionObject(body);
    delete body;
}

