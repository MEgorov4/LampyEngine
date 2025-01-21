#include <gtest/gtest.h>
#include "nlohmann/json.hpp"
#include "../../Modules/ObjectCoreModule/ObjectModel/Node.h"

class TestableNode : public Node 
{
public:
    explicit TestableNode(Node* parentNode = nullptr) : Node(parentNode) {}

    using Node::serialize;
    using Node::deserialize;

    Node*& accessParentNode() {
        return m_parentNode;
    }

    std::vector<Node*>& accessChildNodes() {
        return m_childnodes;
    }

    uint32_t& accessUniqueID() {
        return uniqueID;
    }
};


TEST(NodeTest, AddNode) 
{
    TestableNode* rootNode = new TestableNode();
    TestableNode* childNode = new TestableNode();

    rootNode->addNode(childNode);

    EXPECT_EQ(rootNode->accessChildNodes().size(), 1);
    EXPECT_EQ(rootNode->accessChildNodes()[0], childNode);

    EXPECT_EQ(childNode->accessParentNode(), rootNode);
}

TEST(NodeTest, RemoveNode) 
{
    TestableNode* rootNode = new TestableNode();
    TestableNode* childNode = new TestableNode();

    rootNode->addNode(childNode);
    rootNode->removeNode(childNode);

    EXPECT_EQ(rootNode->accessChildNodes().size(), 0);

    EXPECT_EQ(childNode->accessParentNode(), nullptr);

    delete rootNode;
}

TEST(NodeTest, Serialize)
{
    TestableNode* rootNode = new TestableNode();
    TestableNode* childNode = new TestableNode();
    rootNode->addNode(childNode);

    nlohmann::json json;
    rootNode->serialize(json);

    EXPECT_EQ(json["uniqueID"], rootNode->accessUniqueID());
    EXPECT_EQ(json["children"].size(), 1);
    EXPECT_EQ(json["children"][0]["uniqueID"], childNode->accessUniqueID());

    delete childNode;
}

TEST(NodeTest, Deserialize)
{
    TestableNode* rootNode = new TestableNode();

    nlohmann::json json = {
        {"uniqueID", 42},
        {"children", {
            {{"uniqueID", 43}}
        }}
    };

    rootNode->deserialize(json);

    EXPECT_EQ(rootNode->accessUniqueID(), 42);
    EXPECT_EQ(rootNode->accessChildNodes().size(), 1);
    EXPECT_EQ(rootNode->accessChildNodes()[0]->getUniqueID(), 43);

    delete rootNode;
}


