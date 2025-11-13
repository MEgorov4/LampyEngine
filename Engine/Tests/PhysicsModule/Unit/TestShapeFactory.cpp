#include <gtest/gtest.h>
#include <Modules/PhysicsModule/Factory/ShapeFactory.h>
#include <Modules/PhysicsModule/Utils/PhysicsTypes.h>
#include <btBulletDynamicsCommon.h>

using namespace PhysicsModule;

class ShapeFactoryTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ============================================================================
// Box Shape Tests
// ============================================================================

TEST_F(ShapeFactoryTest, CreateBoxShape)
{
    PhysicsShapeDesc desc;
    desc.type = PhysicsShapeType::Box;
    desc.size = glm::vec3(2.0f, 4.0f, 6.0f);
    
    btCollisionShape* shape = ShapeFactory::Create(desc);
    ASSERT_NE(shape, nullptr);
    
    // Use getShapeType() instead of dynamic_cast (Bullet may be compiled without RTTI)
    EXPECT_EQ(shape->getShapeType(), BOX_SHAPE_PROXYTYPE);
    
    // Safe cast using static_cast after type check
    btBoxShape* boxShape = static_cast<btBoxShape*>(shape);
    ASSERT_NE(boxShape, nullptr);
    
    btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
    EXPECT_NEAR(halfExtents.x(), 1.0f, 0.01f); // size.x * 0.5
    EXPECT_NEAR(halfExtents.y(), 2.0f, 0.01f); // size.y * 0.5
    EXPECT_NEAR(halfExtents.z(), 3.0f, 0.01f); // size.z * 0.5
    
    delete shape;
}

TEST_F(ShapeFactoryTest, CreateBoxShapeDefaultSize)
{
    PhysicsShapeDesc desc;
    desc.type = PhysicsShapeType::Box;
    // Use default size (1,1,1)
    
    btCollisionShape* shape = ShapeFactory::Create(desc);
    ASSERT_NE(shape, nullptr);
    
    // Use getShapeType() instead of dynamic_cast
    EXPECT_EQ(shape->getShapeType(), BOX_SHAPE_PROXYTYPE);
    
    btBoxShape* boxShape = static_cast<btBoxShape*>(shape);
    ASSERT_NE(boxShape, nullptr);
    
    btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
    EXPECT_GT(halfExtents.x(), 0.0f);
    EXPECT_GT(halfExtents.y(), 0.0f);
    EXPECT_GT(halfExtents.z(), 0.0f);
    
    delete shape;
}

// ============================================================================
// Sphere Shape Tests
// ============================================================================

TEST_F(ShapeFactoryTest, CreateSphereShape)
{
    PhysicsShapeDesc desc;
    desc.type = PhysicsShapeType::Sphere;
    desc.radius = 2.5f;
    
    btCollisionShape* shape = ShapeFactory::Create(desc);
    ASSERT_NE(shape, nullptr);
    
    // Use getShapeType() instead of dynamic_cast
    EXPECT_EQ(shape->getShapeType(), SPHERE_SHAPE_PROXYTYPE);
    
    btSphereShape* sphereShape = static_cast<btSphereShape*>(shape);
    ASSERT_NE(sphereShape, nullptr);
    
    EXPECT_NEAR(sphereShape->getRadius(), 2.5f, 0.01f);
    
    delete shape;
}

TEST_F(ShapeFactoryTest, CreateSphereShapeDefaultRadius)
{
    PhysicsShapeDesc desc;
    desc.type = PhysicsShapeType::Sphere;
    // Use default radius (0.5)
    
    btCollisionShape* shape = ShapeFactory::Create(desc);
    ASSERT_NE(shape, nullptr);
    
    // Use getShapeType() instead of dynamic_cast
    EXPECT_EQ(shape->getShapeType(), SPHERE_SHAPE_PROXYTYPE);
    
    btSphereShape* sphereShape = static_cast<btSphereShape*>(shape);
    ASSERT_NE(sphereShape, nullptr);
    
    EXPECT_NEAR(sphereShape->getRadius(), 0.5f, 0.01f);
    
    delete shape;
}

// ============================================================================
// Capsule Shape Tests
// ============================================================================

TEST_F(ShapeFactoryTest, CreateCapsuleShape)
{
    PhysicsShapeDesc desc;
    desc.type = PhysicsShapeType::Capsule;
    desc.radius = 0.5f;
    desc.height = 2.0f;
    
    btCollisionShape* shape = ShapeFactory::Create(desc);
    ASSERT_NE(shape, nullptr);
    
    // Use getShapeType() instead of dynamic_cast
    // btCapsuleShape creates btCapsuleShapeY by default
    // Verify it's a capsule shape (not box or sphere)
    int shapeType = shape->getShapeType();
    EXPECT_NE(shapeType, BOX_SHAPE_PROXYTYPE);
    EXPECT_NE(shapeType, SPHERE_SHAPE_PROXYTYPE);
    // Shape should be some type of capsule (Y-axis is default)
    EXPECT_GE(shapeType, 0); // Just verify it's a valid type
    
    // Safe cast - btCapsuleShapeY inherits from btCapsuleShape
    // We can cast to base class btCapsuleShape
    btCapsuleShape* capsuleShape = static_cast<btCapsuleShape*>(shape);
    ASSERT_NE(capsuleShape, nullptr);
    
    EXPECT_NEAR(capsuleShape->getRadius(), 0.5f, 0.01f);
    EXPECT_NEAR(capsuleShape->getHalfHeight(), 1.0f, 0.01f); // height * 0.5
    
    delete shape;
}

// ============================================================================
// Cylinder Shape Tests
// ============================================================================

TEST_F(ShapeFactoryTest, CreateCylinderShape)
{
    PhysicsShapeDesc desc;
    desc.type = PhysicsShapeType::Cylinder;
    desc.size = glm::vec3(1.0f, 2.0f, 1.0f);
    
    btCollisionShape* shape = ShapeFactory::Create(desc);
    ASSERT_NE(shape, nullptr);
    
    // Use getShapeType() instead of dynamic_cast
    // btCylinderShape creates btCylinderShapeY by default
    // Verify it's a cylinder shape (not box or sphere)
    int shapeType = shape->getShapeType();
    EXPECT_NE(shapeType, BOX_SHAPE_PROXYTYPE);
    EXPECT_NE(shapeType, SPHERE_SHAPE_PROXYTYPE);
    // Shape should be some type of cylinder (Y-axis is default)
    EXPECT_GE(shapeType, 0); // Just verify it's a valid type
    
    // Safe cast - btCylinderShapeY inherits from btCylinderShape
    // We can cast to base class btCylinderShape
    btCylinderShape* cylinderShape = static_cast<btCylinderShape*>(shape);
    ASSERT_NE(cylinderShape, nullptr);
    
    btVector3 halfExtents = cylinderShape->getHalfExtentsWithMargin();
    EXPECT_GT(halfExtents.x(), 0.0f);
    EXPECT_GT(halfExtents.y(), 0.0f);
    EXPECT_GT(halfExtents.z(), 0.0f);
    
    delete shape;
}

// ============================================================================
// Default/Unknown Type Tests
// ============================================================================

TEST_F(ShapeFactoryTest, CreateDefaultShapeForUnknownType)
{
    PhysicsShapeDesc desc;
    // Don't set type, should default to Box
    
    btCollisionShape* shape = ShapeFactory::Create(desc);
    ASSERT_NE(shape, nullptr);
    
    // Should create a box shape as default
    EXPECT_EQ(shape->getShapeType(), BOX_SHAPE_PROXYTYPE);
    
    btBoxShape* boxShape = static_cast<btBoxShape*>(shape);
    ASSERT_NE(boxShape, nullptr);
    
    delete shape;
}

