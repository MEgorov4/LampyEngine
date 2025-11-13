#include <gtest/gtest.h>
#include <Modules/PhysicsModule/Utils/PhysicsConverters.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <btBulletDynamicsCommon.h>

using namespace PhysicsModule;

class PhysicsConvertersTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ============================================================================
// Vector3 Conversion Tests
// ============================================================================

TEST_F(PhysicsConvertersTest, ConvertVec3ToBullet)
{
    glm::vec3 glmVec(1.0f, 2.0f, 3.0f);
    btVector3 btVec = ToBullet(glmVec);
    
    EXPECT_FLOAT_EQ(btVec.x(), 1.0f);
    EXPECT_FLOAT_EQ(btVec.y(), 2.0f);
    EXPECT_FLOAT_EQ(btVec.z(), 3.0f);
}

TEST_F(PhysicsConvertersTest, ConvertVec3FromBullet)
{
    btVector3 btVec(4.0f, 5.0f, 6.0f);
    glm::vec3 glmVec = FromBullet(btVec);
    
    EXPECT_FLOAT_EQ(glmVec.x, 4.0f);
    EXPECT_FLOAT_EQ(glmVec.y, 5.0f);
    EXPECT_FLOAT_EQ(glmVec.z, 6.0f);
}

TEST_F(PhysicsConvertersTest, ConvertVec3RoundTrip)
{
    glm::vec3 original(7.0f, 8.0f, 9.0f);
    btVector3 btVec = ToBullet(original);
    glm::vec3 converted = FromBullet(btVec);
    
    EXPECT_FLOAT_EQ(converted.x, original.x);
    EXPECT_FLOAT_EQ(converted.y, original.y);
    EXPECT_FLOAT_EQ(converted.z, original.z);
}

TEST_F(PhysicsConvertersTest, ConvertVec3Zero)
{
    glm::vec3 zero(0.0f, 0.0f, 0.0f);
    btVector3 btZero = ToBullet(zero);
    glm::vec3 converted = FromBullet(btZero);
    
    EXPECT_FLOAT_EQ(converted.x, 0.0f);
    EXPECT_FLOAT_EQ(converted.y, 0.0f);
    EXPECT_FLOAT_EQ(converted.z, 0.0f);
}

TEST_F(PhysicsConvertersTest, ConvertVec3Negative)
{
    glm::vec3 negative(-1.0f, -2.0f, -3.0f);
    btVector3 btNeg = ToBullet(negative);
    glm::vec3 converted = FromBullet(btNeg);
    
    EXPECT_FLOAT_EQ(converted.x, -1.0f);
    EXPECT_FLOAT_EQ(converted.y, -2.0f);
    EXPECT_FLOAT_EQ(converted.z, -3.0f);
}

// ============================================================================
// Quaternion Conversion Tests
// ============================================================================

TEST_F(PhysicsConvertersTest, ConvertQuatToBullet)
{
    glm::quat glmQuat(1.0f, 0.0f, 0.0f, 0.0f); // Identity
    btQuaternion btQuat = ToBullet(glmQuat);
    
    EXPECT_FLOAT_EQ(btQuat.w(), 1.0f);
    EXPECT_FLOAT_EQ(btQuat.x(), 0.0f);
    EXPECT_FLOAT_EQ(btQuat.y(), 0.0f);
    EXPECT_FLOAT_EQ(btQuat.z(), 0.0f);
}

TEST_F(PhysicsConvertersTest, ConvertQuatFromBullet)
{
    // Bullet quaternion constructor: btQuaternion(x, y, z, w)
    // So (0.707f, 0.0f, 0.0f, 0.707f) means x=0.707, y=0, z=0, w=0.707
    btQuaternion btQuat(0.707f, 0.0f, 0.0f, 0.707f);
    glm::quat glmQuat = FromBullet(btQuat);
    
    // GLM quaternion order: (w, x, y, z)
    EXPECT_NEAR(glmQuat.w, 0.707f, 0.001f);
    EXPECT_NEAR(glmQuat.x, 0.707f, 0.001f);
    EXPECT_NEAR(glmQuat.y, 0.0f, 0.001f);
    EXPECT_NEAR(glmQuat.z, 0.0f, 0.001f);
}

TEST_F(PhysicsConvertersTest, ConvertQuatRoundTrip)
{
    glm::quat original(0.5f, 0.5f, 0.5f, 0.5f);
    btQuaternion btQuat = ToBullet(original);
    glm::quat converted = FromBullet(btQuat);
    
    EXPECT_NEAR(converted.w, original.w, 0.001f);
    EXPECT_NEAR(converted.x, original.x, 0.001f);
    EXPECT_NEAR(converted.y, original.y, 0.001f);
    EXPECT_NEAR(converted.z, original.z, 0.001f);
}

// ============================================================================
// Transform Conversion Tests
// ============================================================================

TEST_F(PhysicsConvertersTest, ConvertTransformToBullet)
{
    glm::vec3 pos(1.0f, 2.0f, 3.0f);
    glm::quat rot(1.0f, 0.0f, 0.0f, 0.0f);
    btTransform transform = ToBullet(pos, rot);
    
    btVector3 origin = transform.getOrigin();
    EXPECT_FLOAT_EQ(origin.x(), 1.0f);
    EXPECT_FLOAT_EQ(origin.y(), 2.0f);
    EXPECT_FLOAT_EQ(origin.z(), 3.0f);
}

TEST_F(PhysicsConvertersTest, ConvertTransformFromBullet)
{
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(4.0f, 5.0f, 6.0f));
    
    glm::vec3 pos;
    glm::quat rot;
    FromBullet(transform, pos, rot);
    
    EXPECT_FLOAT_EQ(pos.x, 4.0f);
    EXPECT_FLOAT_EQ(pos.y, 5.0f);
    EXPECT_FLOAT_EQ(pos.z, 6.0f);
}

TEST_F(PhysicsConvertersTest, ConvertTransformRoundTrip)
{
    glm::vec3 originalPos(7.0f, 8.0f, 9.0f);
    glm::quat originalRot(0.707f, 0.707f, 0.0f, 0.0f);
    
    btTransform transform = ToBullet(originalPos, originalRot);
    
    glm::vec3 convertedPos;
    glm::quat convertedRot;
    FromBullet(transform, convertedPos, convertedRot);
    
    EXPECT_NEAR(convertedPos.x, originalPos.x, 0.001f);
    EXPECT_NEAR(convertedPos.y, originalPos.y, 0.001f);
    EXPECT_NEAR(convertedPos.z, originalPos.z, 0.001f);
}

