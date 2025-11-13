#include <gtest/gtest.h>
#include <Modules/PhysicsModule/Utils/PhysicsTypes.h>

using namespace PhysicsModule;

class PhysicsTypesTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ============================================================================
// PhysicsShapeType Tests
// ============================================================================

TEST_F(PhysicsTypesTest, PhysicsShapeTypeValues)
{
    EXPECT_EQ(static_cast<uint8_t>(PhysicsShapeType::Box), 0);
    EXPECT_EQ(static_cast<uint8_t>(PhysicsShapeType::Sphere), 1);
    EXPECT_EQ(static_cast<uint8_t>(PhysicsShapeType::Capsule), 2);
    EXPECT_EQ(static_cast<uint8_t>(PhysicsShapeType::Cylinder), 3);
    EXPECT_EQ(static_cast<uint8_t>(PhysicsShapeType::Mesh), 4);
    EXPECT_EQ(static_cast<uint8_t>(PhysicsShapeType::ConvexHull), 5);
}

// ============================================================================
// RigidBodyType Tests
// ============================================================================

TEST_F(PhysicsTypesTest, RigidBodyTypeValues)
{
    EXPECT_EQ(static_cast<uint8_t>(RigidBodyType::Static), 0);
    EXPECT_EQ(static_cast<uint8_t>(RigidBodyType::Dynamic), 1);
    EXPECT_EQ(static_cast<uint8_t>(RigidBodyType::Kinematic), 2);
}

// ============================================================================
// PhysicsShapeDesc Tests
// ============================================================================

TEST_F(PhysicsTypesTest, PhysicsShapeDescDefaultValues)
{
    PhysicsShapeDesc desc;
    
    EXPECT_EQ(desc.type, PhysicsShapeType::Box);
    EXPECT_FLOAT_EQ(desc.size.x, 1.0f);
    EXPECT_FLOAT_EQ(desc.size.y, 1.0f);
    EXPECT_FLOAT_EQ(desc.size.z, 1.0f);
    EXPECT_FLOAT_EQ(desc.radius, 0.5f);
    EXPECT_FLOAT_EQ(desc.height, 1.0f);
}

TEST_F(PhysicsTypesTest, PhysicsShapeDescCustomValues)
{
    PhysicsShapeDesc desc;
    desc.type = PhysicsShapeType::Sphere;
    desc.size = glm::vec3(2.0f, 2.0f, 2.0f);
    desc.radius = 1.0f;
    desc.height = 2.0f;
    
    EXPECT_EQ(desc.type, PhysicsShapeType::Sphere);
    EXPECT_FLOAT_EQ(desc.size.x, 2.0f);
    EXPECT_FLOAT_EQ(desc.radius, 1.0f);
    EXPECT_FLOAT_EQ(desc.height, 2.0f);
}

// ============================================================================
// RigidBodyDesc Tests
// ============================================================================

TEST_F(PhysicsTypesTest, RigidBodyDescDefaultValues)
{
    RigidBodyDesc desc;
    
    EXPECT_FLOAT_EQ(desc.mass, 1.0f);
    EXPECT_EQ(desc.bodyType, RigidBodyType::Dynamic);
    EXPECT_FLOAT_EQ(desc.position.x, 0.0f);
    EXPECT_FLOAT_EQ(desc.position.y, 0.0f);
    EXPECT_FLOAT_EQ(desc.position.z, 0.0f);
    EXPECT_FLOAT_EQ(desc.rotation.w, 1.0f); // Identity quaternion
}

TEST_F(PhysicsTypesTest, RigidBodyDescCustomValues)
{
    RigidBodyDesc desc;
    desc.mass = 5.0f;
    desc.bodyType = RigidBodyType::Static;
    desc.position = glm::vec3(10.0f, 20.0f, 30.0f);
    desc.rotation = glm::quat(0.707f, 0.707f, 0.0f, 0.0f);
    
    EXPECT_FLOAT_EQ(desc.mass, 5.0f);
    EXPECT_EQ(desc.bodyType, RigidBodyType::Static);
    EXPECT_FLOAT_EQ(desc.position.x, 10.0f);
    EXPECT_FLOAT_EQ(desc.position.y, 20.0f);
    EXPECT_FLOAT_EQ(desc.position.z, 30.0f);
}

// ============================================================================
// PhysicsMaterialDesc Tests
// ============================================================================

TEST_F(PhysicsTypesTest, PhysicsMaterialDescDefaultValues)
{
    PhysicsMaterialDesc desc;
    
    EXPECT_FLOAT_EQ(desc.friction, 0.5f);
    EXPECT_FLOAT_EQ(desc.restitution, 0.0f);
    EXPECT_FLOAT_EQ(desc.density, 1.0f);
}

TEST_F(PhysicsTypesTest, PhysicsMaterialDescCustomValues)
{
    PhysicsMaterialDesc desc;
    desc.friction = 0.8f;
    desc.restitution = 0.5f;
    desc.density = 2.5f;
    
    EXPECT_FLOAT_EQ(desc.friction, 0.8f);
    EXPECT_FLOAT_EQ(desc.restitution, 0.5f);
    EXPECT_FLOAT_EQ(desc.density, 2.5f);
}

// ============================================================================
// RaycastHit Tests
// ============================================================================

TEST_F(PhysicsTypesTest, RaycastHitDefaultValues)
{
    RaycastHit hit;
    
    EXPECT_FALSE(hit.hit);
    EXPECT_FLOAT_EQ(hit.distance, 0.0f);
    EXPECT_FLOAT_EQ(hit.point.x, 0.0f);
    EXPECT_FLOAT_EQ(hit.point.y, 0.0f);
    EXPECT_FLOAT_EQ(hit.point.z, 0.0f);
}

// ============================================================================
// Handle Tests
// ============================================================================

TEST_F(PhysicsTypesTest, HandleInvalidValues)
{
    EXPECT_EQ(InvalidBodyHandle, 0);
    EXPECT_EQ(InvalidShapeHandle, 0);
}

