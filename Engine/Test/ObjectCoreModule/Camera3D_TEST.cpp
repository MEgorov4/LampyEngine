#include <gtest/gtest.h>
#include "../../Modules/ObjectCoreModule/ObjectModel/3D/Camera3D.h"

TEST(Camera3DTest, DefaultConstructor)
{
    Camera3D camera;

    EXPECT_EQ(camera.getProjectionMatrix(), glm::perspective(glm::radians(60.f), 16.0f / 9.0f, 0.1f, 1000.f));

    glm::mat4 expectedViewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    EXPECT_EQ(camera.getViewMatrix(), expectedViewMatrix);
}

TEST(Camera3DTest, SetPerspective)
{
    Camera3D camera;

    float fov = 75.0f;
    float aspect = 4.0f / 3.0f;
    float nearPlane = 0.5f;
    float farPlane = 500.0f;

    camera.setPerspective(fov, aspect, nearPlane, farPlane);

    glm::mat4 expectedProjection = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    EXPECT_EQ(camera.getProjectionMatrix(), expectedProjection);
}

TEST(Camera3DTest, UpdateViewMatrix_PositionChange)
{
    Camera3D camera;

    glm::vec3 newPosition(10.0f, 5.0f, -10.0f);
    camera.setPosition(newPosition);
    camera.updateViewMatrix();

    glm::mat4 expectedViewMatrix = glm::lookAt(newPosition, newPosition + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    EXPECT_EQ(camera.getViewMatrix(), expectedViewMatrix);
}

TEST(Camera3DTest, UpdateViewMatrix_RotationChange)
{
    Camera3D camera;

    camera.setRotation(glm::vec3(0.0f, 90.0f, 0.0f)); 
    camera.updateViewMatrix();

    glm::mat4 rotation = glm::yawPitchRoll(glm::radians(90.0f), glm::radians(0.0f), glm::radians(0.0f));
    glm::vec3 forward = glm::normalize(glm::vec3(rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::vec3(rotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

    glm::mat4 expectedViewMatrix = glm::lookAt(camera.getPosition(), camera.getPosition() + forward, up);
    EXPECT_EQ(camera.getViewMatrix(), expectedViewMatrix);
}

TEST(Camera3DTest, GlobalTransform_WithParentNode)
{
    Node3D parentNode;
    parentNode.setPosition(glm::vec3(10.0f, 0.0f, 0.0f));

    Camera3D camera(&parentNode);
    camera.setPosition(glm::vec3(5.0f, 0.0f, 0.0f));
    camera.updateViewMatrix();

    glm::mat4 parentGlobalTransform = parentNode.getGlobalTransform();
    glm::mat4 cameraLocalTransform = camera.getLocalTransform();
    glm::mat4 expectedGlobalTransform = parentGlobalTransform * cameraLocalTransform;

    EXPECT_EQ(camera.getGlobalTransform(), expectedGlobalTransform);
}

TEST(Camera3DTest, ComplexTransformHierarchy)
{
    Node3D rootNode;
    rootNode.setPosition(glm::vec3(10.0f, 0.0f, 0.0f));

    Node3D intermediateNode(&rootNode);
    intermediateNode.setPosition(glm::vec3(5.0f, 5.0f, 0.0f));

    Camera3D camera(&intermediateNode);
    camera.setPosition(glm::vec3(0.0f, 0.0f, -5.0f));
    camera.updateViewMatrix();

    glm::mat4 expectedGlobalTransform = rootNode.getGlobalTransform() *
        intermediateNode.getLocalTransform() *
        camera.getLocalTransform();

    EXPECT_EQ(camera.getGlobalTransform(), expectedGlobalTransform);

    glm::vec3 expectedGlobalPosition = glm::vec3(expectedGlobalTransform[3]);
    EXPECT_EQ(camera.getGlobalPosition(), expectedGlobalPosition);
}

TEST(Camera3DTest, ViewProjectionCombined)
{
    Camera3D camera;

    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix();

    glm::mat4 viewProjection = projection * view;

    EXPECT_EQ(viewProjection, camera.getProjectionMatrix() * camera.getViewMatrix());
}
