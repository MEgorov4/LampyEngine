#include <gtest/gtest.h>
#include "../../Modules/ObjectCoreModule/ObjectModel/3D/Camera3D.h"


TEST(Node3DTest, Constructor)
{
    Node3D node;
    EXPECT_EQ(node.getPosition(), glm::vec3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(node.getRotation(), glm::vec3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(node.getScale(), glm::vec3(1.0f, 1.0f, 1.0f));
}

TEST(Node3DTest, SetAndGetTransformations)
{
    Node3D node;

    node.setPosition(glm::vec3(10.0f, 20.0f, 30.0f));
    EXPECT_EQ(node.getPosition(), glm::vec3(10.0f, 20.0f, 30.0f));

    node.setRotation(glm::vec3(45.0f, 90.0f, 180.0f));
    EXPECT_EQ(node.getRotation(), glm::vec3(45.0f, 90.0f, 180.0f));

    node.setScale(glm::vec3(2.0f, 2.0f, 2.0f));
    EXPECT_EQ(node.getScale(), glm::vec3(2.0f, 2.0f, 2.0f));
}

TEST(Node3DTest, LocalTransform)
{
    Node3D node;

    node.setPosition(glm::vec3(10.0f, 0.0f, 0.0f));
    node.setRotation(glm::vec3(0.0f, 90.0f, 0.0f));
    node.setScale(glm::vec3(2.0f, 2.0f, 2.0f));

    glm::mat4 localTransform = node.getLocalTransform();

    glm::mat4 expectedTransform = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));

    EXPECT_EQ(localTransform, expectedTransform);
}

TEST(Node3DTest, GlobalTransform_NoParent)
{
    Node3D node;

    node.setPosition(glm::vec3(5.0f, 10.0f, 15.0f));
    glm::mat4 globalTransform = node.getGlobalTransform();

    EXPECT_EQ(globalTransform, node.getLocalTransform());
}

TEST(Node3DTest, GlobalTransform_WithParent)
{
    Node3D parentNode;
    parentNode.setPosition(glm::vec3(10.0f, 20.0f, 30.0f));

    Node3D childNode(&parentNode);
    childNode.setPosition(glm::vec3(5.0f, 0.0f, 0.0f));

    glm::mat4 expectedTransform = parentNode.getGlobalTransform() * childNode.getLocalTransform();
    EXPECT_EQ(childNode.getGlobalTransform(), expectedTransform);
}

TEST(Node3DTest, GlobalPosition)
{
    Node3D parentNode;
    parentNode.setPosition(glm::vec3(10.0f, 20.0f, 30.0f));

    Node3D childNode(&parentNode);
    childNode.setPosition(glm::vec3(5.0f, 0.0f, 0.0f));

    EXPECT_EQ(childNode.getGlobalPosition(), glm::vec3(15.0f, 20.0f, 30.0f));
}

TEST(Node3DTest, GlobalRotation)
{
    Node3D parentNode;
    parentNode.setRotation(glm::vec3(30.0f, 45.0f, 60.0f));

    Node3D childNode(&parentNode);
    childNode.setRotation(glm::vec3(10.0f, 20.0f, 30.0f));

    EXPECT_EQ(childNode.getGlobalRotation(), glm::vec3(40.0f, 65.0f, 90.0f));
}

TEST(Node3DTest, GlobalScale)
{
    Node3D parentNode;
    parentNode.setScale(glm::vec3(2.0f, 2.0f, 2.0f));

    Node3D childNode(&parentNode);
    childNode.setScale(glm::vec3(0.5f, 0.5f, 0.5f));

    EXPECT_EQ(childNode.getGlobalScale(), glm::vec3(1.0f, 1.0f, 1.0f));
}

TEST(Node3DTest, ComplexHierarchy)
{
    Node3D rootNode;
    rootNode.setPosition(glm::vec3(10.0f, 0.0f, 0.0f));

    Node3D childNode1(&rootNode);
    childNode1.setPosition(glm::vec3(0.0f, 5.0f, 0.0f));

    Node3D childNode2(&childNode1);
    childNode2.setPosition(glm::vec3(0.0f, 0.0f, 3.0f));

    EXPECT_EQ(childNode2.getGlobalPosition(), glm::vec3(10.0f, 5.0f, 3.0f));
}

